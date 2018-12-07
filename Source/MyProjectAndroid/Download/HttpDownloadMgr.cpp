// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "HttpDownloadMgr.h"
#include "AsyncHttpFile.h"

FHttpDownloadMgr * FHttpDownloadMgr::DownloadManager = NULL;

FHttpDownloadMgr::FHttpDownloadMgr()
	:MaxParallel(5)
	,MaxTryCount(100)
	,RequestKBSize(500)
	,ResponseTimeout(10.0f)
{
}

UAsyncHttpFile *UAsyncHttpFile::LoadHttpFile(UObject *WorldContextObject, const FString &Url, const FString &FileName, const FString &Directory)
{	
	FHttpDownloadMgr *Mgr = FHttpDownloadMgr::GetDownloadMgr();
	UAsyncHttpFile *Task = NULL;
	
	if (Mgr)
	{
		Task = Mgr->CreateTask(WorldContextObject, Url, FileName, Directory);
		Mgr->UpdateHttpFiles();
	}

	return Task;
}

void UAsyncHttpFile::SetMaxParallel(int32 NumParallel)
{
	FHttpDownloadMgr *Mgr = FHttpDownloadMgr::GetDownloadMgr();
	if (Mgr)
	{
		Mgr->SetMaxParallel(NumParallel);
	}
}

void UAsyncHttpFile::SetMaxTryCount(int32 TryCount)
{
	FHttpDownloadMgr *Mgr = FHttpDownloadMgr::GetDownloadMgr();
	if (Mgr)
	{
		Mgr->SetMaxTryCount(TryCount);
	}
}

void UAsyncHttpFile::SetURLRequestKbSize(int32 NumKB)
{
	FHttpDownloadMgr *Mgr = FHttpDownloadMgr::GetDownloadMgr();
	if (Mgr)
	{
		Mgr->SetURLRequestKbSize(NumKB);
	}
}

void UAsyncHttpFile::SetTimeOut(float Timeout)
{
	FHttpDownloadMgr *Mgr = FHttpDownloadMgr::GetDownloadMgr();
	if (Mgr)
	{
		Mgr->SetTimeOut(Timeout);
	}
}

void FHttpDownloadMgr::SetMaxParallel(int32 NumTask)
{
	MaxParallel = NumTask;
	UpdateHttpFiles();
}

void FHttpDownloadMgr::SetMaxTryCount(int32 TryCount)
{
	MaxTryCount = TryCount;
	UpdateHttpFiles();
}

void FHttpDownloadMgr::SetURLRequestKbSize(int32 NumKB)
{
	if (NumKB > 0)
	{
		RequestKBSize = NumKB;
	}
}

void FHttpDownloadMgr::SetTimeOut(float Timeout)
{
	if (Timeout > 0)
	{
		ResponseTimeout = Timeout;
	}
}

FHttpDownloadMgr *FHttpDownloadMgr::GetDownloadMgr()
{
	if (!DownloadManager)
	{
		DownloadManager = new FHttpDownloadMgr();
	}
	return DownloadManager;
}

UAsyncHttpFile *FHttpDownloadMgr::CreateTask(UObject *WorldContextObject, const FString &Url, const FString &FileName, const FString &Directory)
{
	UAsyncHttpFile *HttpFile = NewObject<UAsyncHttpFile>(WorldContextObject, *(Directory / FileName));
	
	if (HttpFile)
	{
		HttpFile->Url = Url;
		HttpFile->FileName = FileName;
		HttpFile->Directory = Directory;
		HttpFile->State = EHFTaskState::EPending;
		HttpFile->Mgr = this;
		DownloadHttpFiles.AddUnique(HttpFile);
	}

	return HttpFile;
}

void FHttpDownloadMgr::NotifyFailed(UAsyncHttpFile *HttpFile, bool bForceFailed)
{
	if (!bForceFailed)
	{
		if (HttpFile)
		{
			if (HttpFile->GetTryCount() < MaxTryCount)
			{
				HttpFile->NotifyWait();
			}
			else
			{
				HttpFile->NotifyFailed();
			}
		}
	}
	UpdateHttpFiles();
}

void FHttpDownloadMgr::NotifyFinised(UAsyncHttpFile *HttpFile)
{
	UpdateHttpFiles();
}

void FHttpDownloadMgr::UpdateHttpFiles()
{
	int32 NumActiveTask = 0;
	TArray<UAsyncHttpFile *> ProbTasks;

	for (int32 i = DownloadHttpFiles.Num()-1; i >= 0; --i)
	{
		UAsyncHttpFile *HttpFile = DownloadHttpFiles[i];
		if (HttpFile)
		{
			EHFTaskState State = HttpFile->GetState();
			
			if (State == EHFTaskState::ESuccees)
			{
				DownloadHttpFiles.RemoveAt(i);
				continue;
			}
			else if (State == EHFTaskState::EWaitTry)
			{
				if (HttpFile->GetTryCount() < MaxTryCount)
				{
					ProbTasks.Add(HttpFile);
				}
			}
			else if (State == EHFTaskState::EPending)
			{
				ProbTasks.Add(HttpFile);
			}
			else if (State == EHFTaskState::EDownloading)
			{
				NumActiveTask++;
			}
		}
		else
		{
			DownloadHttpFiles.RemoveAt(i);
			continue;
		}
	}

	if (NumActiveTask < MaxParallel && ProbTasks.Num()>0)
	{
		UAsyncHttpFile *HttpFile = ProbTasks[0];
		HttpFile->StartTask();
	}
}

void  FHttpDownloadMgr::Tick(float DeltaTime)
{
	double RealTime = FApp::GetCurrentTime();

	for (int32 i = DownloadHttpFiles.Num() - 1; i>=0; --i)
	{
		UAsyncHttpFile *HttpFile = DownloadHttpFiles[i];
		if (HttpFile)
		{
			if (HttpFile->GetState() == EHFTaskState::EDownloading)
			{
				HttpFile->CheckDeadTask(RealTime);
			}
		}
		else
		{
			DownloadHttpFiles.RemoveAt(i);
		}
	}
}

bool FHttpDownloadMgr::IsTickable() const
{
	return DownloadHttpFiles.Num()>0;
}

void FHttpDownloadMgr::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects(DownloadHttpFiles);
}

void FHttpDownloadMgr::ClearMgr()
{
	for (int32 i = DownloadHttpFiles.Num() - 1; i >= 0; --i) 
	{
		DownloadHttpFiles.RemoveAt(i);
		continue;
	}
}