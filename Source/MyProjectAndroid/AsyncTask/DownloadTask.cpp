
#include "DownloadTask.h"
#include "ResourceMgr.h"
#include "HttpModule.h"
#include "ModelFile.h"
#include "AsyncHttpFile.h"
#include "HttpDownloadMgr.h"
#include "Misc/ScopeLock.h"

FDownloadAsyncTask::FDownloadAsyncTask(UResourceMgr *InResMgr, EResType InResType, const FString &InResID, const FString &InURL, const FString &InFilename, const TArray<FDownloadingRes> &DependsResources, FOnDownloadTaskCompeleted InDelegate, int32 InSvrVersion /*= 0*/)
	:Delegate(InDelegate)
	,URL(InURL)
	,NumLoaded(0)
	,NumFailed(0)
	,Callback(NULL)
	,ResType(InResType)
	,ResID(InResID)
	,ResMgr(InResMgr)
	,SrvVersion(InSvrVersion)
{
	DownloadRes.Add(FDownloadingRes(InResType, InURL, InFilename));
	DownloadRes.Append(DependsResources);		
}

void FDownloadAsyncTask::DoWork()
{
	if (ResMgr)
	{
		FScopeLock Lock(&MutexDownload);

		Callback = NewObject<UDownloadCallback>();
		Callback->Task = this;

		for (int32 i = DownloadRes.Num() - 1; i >= 0; --i)
		{
			if (StopCouter.GetValue() > 0)
			{
				break;
			}

			FDownloadingRes &Res = DownloadRes[i];
			UResource *FoundRes = ResMgr->FindResByFilename(Res.Filename, true);
			if (FoundRes && !FoundRes->IsNeedUpgrade(SrvVersion))
			{
				Res.Resource = FoundRes;
				Res.bNeedDownload = false;
				if (FoundRes->IsLoaded())
				{
					OnLoadCompleted(true, FoundRes);
				}
				else
				{
					FoundRes->StateChanged.AddDynamic(Callback, &UDownloadCallback::OnResStatedChanged);
					if (FoundRes->IsNeedLoad())
					{
						ResMgr->LoadResByFile(Res.Filename, true);
					}
				}
			}
			else
			{
				Res.bNeedDownload = true;
				FString Filename = FPaths::GetCleanFilename(Res.Filename);
				FString Dir = FPaths::GetPath(Filename);
				UAsyncHttpFile *AsyncHttpFile = UAsyncHttpFile::LoadHttpFile(ResMgr, Res.URL, Filename, Dir);
				if (AsyncHttpFile)
				{
					Res.HttpFile = AsyncHttpFile;
					AsyncHttpFile->OnSuccess.AddDynamic(Callback, &UDownloadCallback::OnHttpDownloadSuccess);
					AsyncHttpFile->OnFail.AddDynamic(Callback, &UDownloadCallback::OnHttpDownloadFailed);
					AsyncHttpFile->OnProgress.AddDynamic(Callback, &UDownloadCallback::OnHttpProgress);
				}
			}
		}
	}
}

void FDownloadAsyncTask::Stop()
{
	FScopeLock Lock(&MutexDownload);
	StopCouter.Increment();

	for (int32 i = DownloadRes.Num() - 1; i >= 0; --i)
	{
		FDownloadingRes &Res = DownloadRes[i];
		if (Res.HttpFile)
		{
			Res.HttpFile->Stop();
			Res.HttpFile->ConditionalBeginDestroy();
			Res.HttpFile = nullptr;
		}
	}
}

bool FDownloadAsyncTask::IsTaskDone()
{
	return StopCouter.GetValue()>0 || (NumFailed + NumLoaded) >= DownloadRes.Num();
}

void FDownloadAsyncTask::ExecuteDone()
{
	if (StopCouter.GetValue() <= 0)
	{
		for (int32 i = DownloadRes.Num() - 1; i >= 0; --i)
		{
			FDownloadingRes &Res = DownloadRes[i];
			if (Res.bNeedDownload)
			{
				UResource *Resoruce = Res.Resource;
				if (!Resoruce)
				{
					Resoruce = ResMgr->FindResByFilename(Res.Filename, true);
				}

				if (Resoruce)
				{
					Resoruce->OnDownloaded();
					Resoruce->Save();
				}
			}
		}

		if (ResMgr)
		{
			int32 Num = DownloadRes.Num();
			UResource *MainResource = ResMgr->FindRes(ResID, true);
			if (MainResource)
			{
				Delegate.ExecuteIfBound(MainResource, (float(NumFailed + NumLoaded) / float(Num)), true);
			}
		}
	}

	DownloadRes.Empty();
}

void FDownloadAsyncTask::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (ResMgr)
	{
		Collector.AddReferencedObject(ResMgr);
	}

	if (Callback)
	{
		Collector.AddReferencedObject(Callback);
	}

	for (int32 i = 0; i < DownloadRes.Num(); ++i)
	{
		FDownloadingRes &Res = DownloadRes[i];
		
		if (Res.Resource)
		{
			Collector.AddReferencedObject(Res.Resource);
		}

		if (Res.HttpFile)
		{
			Collector.AddReferencedObject(Res.HttpFile);
		}
	}
}

void FDownloadAsyncTask::NotifyHttpDownloadSuccess(UAsyncHttpFile *HttpFile, const FString &Filename)
{
	if (ResMgr)
	{
		UResource *FoundRes = ResMgr->LoadResByFile(Filename, false);
		if (FoundRes)
		{
			if (FoundRes->IsHeadLoaded())
			{
				ResMgr->FreeResource(FoundRes);
			}
			
			FoundRes = ResMgr->LoadResByFile(Filename, true);
			if (FoundRes)
			{
				FoundRes->StateChanged.AddDynamic(Callback, &UDownloadCallback::OnResStatedChanged);
			}
		}
		else
		{
			OnLoadCompleted(false, NULL);
		}
	}
}

void FDownloadAsyncTask::NotifyHttpDownloadFailed(UAsyncHttpFile *HttpFile)
{
	OnLoadCompleted(false, NULL);
}

void FDownloadAsyncTask::NotifyHttpProgress(UAsyncHttpFile *HttpFile, float Progress)
{
	int32 Num = DownloadRes.Num();
	Delegate.ExecuteIfBound(NULL, (float(NumFailed + NumLoaded) + Progress)/float(Num), false);
}

void FDownloadAsyncTask::OnLoadCompleted(bool bSuccess, UResource *Resource)
{
	if(bSuccess)
	{ 
		NumLoaded++;
	}
	else
	{
		NumFailed++;
	}

	if (DownloadRes.Num() > 0)
	{
		int32 Num = DownloadRes.Num();
		Delegate.ExecuteIfBound(Resource, float(NumFailed + NumLoaded) / float(Num), false);
	}
}

//////////////////////////////////////////////////////////////////////////
void UDownloadCallback::OnResStatedChanged(UResource * Resource, EResState State, UVaRestJsonObject * DRInfo)
{
	if (Resource)
	{
		Resource->StateChanged.RemoveDynamic(this, &UDownloadCallback::OnResStatedChanged);
		if (Task)
		{
			if (State == EResState::EResLoaded)
			{
				Task->OnLoadCompleted(true, Resource);
			}
			else if (State == EResState::EResFailed)
			{
				Task->OnLoadCompleted(false, Resource);
			}
		}
	}
}

void UDownloadCallback::OnHttpDownloadSuccess(UAsyncHttpFile *HttpFile, const FString &DestinationPath, float DownloadPercent)
{
	if (HttpFile)
	{
		if (Task)
		{
			Task->NotifyHttpDownloadSuccess(HttpFile, DestinationPath);
		}

		HttpFile->OnSuccess.RemoveDynamic(this, &UDownloadCallback::OnHttpDownloadSuccess);
		HttpFile->OnFail.RemoveDynamic(this, &UDownloadCallback::OnHttpDownloadFailed);
		HttpFile->OnProgress.RemoveDynamic(this, &UDownloadCallback::OnHttpProgress);
	}
}

void UDownloadCallback::OnHttpDownloadFailed(UAsyncHttpFile *HttpFile, const FString &DestinationPath, float DownloadPercent)
{
	if (HttpFile)
	{
		if (Task)
		{
			Task->NotifyHttpDownloadFailed(HttpFile);
		}
		HttpFile->OnSuccess.RemoveDynamic(this, &UDownloadCallback::OnHttpDownloadSuccess);
		HttpFile->OnFail.RemoveDynamic(this, &UDownloadCallback::OnHttpDownloadFailed);
		HttpFile->OnProgress.RemoveDynamic(this, &UDownloadCallback::OnHttpProgress);
	}
}

void UDownloadCallback::OnHttpProgress(UAsyncHttpFile * HttpFile, const FString &DestinationPath, float DownloadPercent)
{
	if (HttpFile && Task)
	{
		Task->NotifyHttpProgress(HttpFile, DownloadPercent);
	}
}
