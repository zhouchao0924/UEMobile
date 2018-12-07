
#include "LoadTask.h"
#include "ResourceMgr.h"
#include "HttpModule.h"
#include "ModelFile.h"
#include "Interfaces/IHttpRequest.h"

FResLoadAsyncTask::FResLoadAsyncTask(UResource *Resource)
{
	UResourceMgr *ResMgr = Resource ? Resource->ResMgr : NULL;

	if (ResMgr && Resource)
	{
		TArray<UResource *> ResStack;

		if (Resource->IsNeedLoad())
		{
			ResStack.Add(Resource);
		}

		while (ResStack.Num() > 0)
		{
			UResource *CurRes = ResStack.Pop();

			PendLoadingResources.Add(CurRes);
			CurRes->SetPendingLoad();

			FResourceSummary *Summary = CurRes->GetSummary();
			if (Summary)
			{
				for (int32 i = 0; i < Summary->Dependences.Num(); ++i)
				{
					FString &Dep = Summary->Dependences[i];
					UResource *DepRes = ResMgr->FindRes(Dep);
					if (DepRes && DepRes->IsNeedLoad())
					{
						ResStack.Add(DepRes);
						DepRes->SetPendingLoad();
					}
				}
			}
		}
	}
	PendingResource = Resource;
}

void FResLoadAsyncTask::DoWork()
{
	for (int32 i = PendLoadingResources.Num()-1; i>=0; --i)
	{
		UResource *Res = PendLoadingResources[i];
		if (Res!=NULL)
		{
			FArchive *Writer = IFileManager::Get().CreateFileReader(*Res->Filename);
			if (Writer && UResource::SkipMark(*Writer))
			{
				Res->Serialize(*Writer);
				Res->OnPostLoad();
				Writer->Close();
			}
		}
	}
}

void FResLoadAsyncTask::ExecuteDone()
{
	if (PendingResource!=NULL)
	{
		PendingResource->SetState(EResState::EResLoaded);
	}
}

void FResLoadAsyncTask::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PendingResource);
	Collector.AddReferencedObjects(PendLoadingResources);
}



