// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "AsyncHttpFile.h"
#include "HttpModule.h"
#include "HttpDownloadMgr.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/App.h"

FSubHttpTask::FSubHttpTask()
	:TaskID(INDEX_NONE)
	,bFinished(false)
	,StarPos(0)
	,Size(0)
	,bWaitResponse(false)
{
}

TSharedPtr<IHttpRequest> FSubHttpTask::CreateRequest()
{
	Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader("Range", Range);

	return Request;
}

void FSubHttpTask::Stop()
{
	if (Request.IsValid())
	{
		Request->CancelRequest();
		Request = nullptr;
	}
}

UAsyncHttpFile::UAsyncHttpFile(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	,TryCount(0)
	,Progress(0)
	,CurFileSize(0)
	,TotalFileSize(0)
	,MaxTask(2)
{
}

void UAsyncHttpFile::StartTask()
{
	check(TotalFileSize <= 0);
	State = EHFTaskState::EDownloading;

	FString NewDirectorPath = Directory;
	IFileManager::Get().MakeDirectory(*NewDirectorPath, true);
	
	if (Url.Len() > 0 && FileName.Len() > 0)
	{
		RequestFileSize();
	}
	else {
		FatalErr("URL and FileName must set");
	}

	TryCount++;
}

void UAsyncHttpFile::RequestFileSize()
{
	FileSizeRequest = FHttpModule::Get().CreateRequest();
	FileSizeRequest->OnProcessRequestComplete().BindUObject(this, &UAsyncHttpFile::HandleResponseFileSize);
	FileSizeRequest->SetURL(Url);
	FileSizeRequest->SetVerb(TEXT("GET"));
	FileSizeRequest->SetHeader("Range", "bytes=0-1");
	FileSizeRequest->ProcessRequest();
}

void UAsyncHttpFile::HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (TotalFileSize <= 0)
	{
		if (HttpResponse.IsValid())
		{
			int32 ResponseCode = HttpResponse->GetResponseCode();
			if (ResponseCode / 200 == 1)
			{
				FString ContentRangeStr = HttpResponse->GetHeader("Content-Range");
				FString SplitLeftStr;
				FString SplitRightStr;
				ContentRangeStr.Split("/", &SplitLeftStr, &SplitRightStr);
				if (SplitRightStr.Len() > 0)
				{
					TotalFileSize = FCString::Atoi(*SplitRightStr);
				}
			}
		}

		FileSizeRequest = nullptr;

		if (TotalFileSize <= 0)
		{
			Mgr->NotifyFailed(this, false);
		}
		else
		{
			BeginDownload();
		}
	}
}

void UAsyncHttpFile::CreateSubTask()
{
	const int32 TaskSize = Mgr->RequestKBSize * 1024;
	
	int32 StarPos = 0;
	SubTasks.Empty();
	int32 TaskID = 0;

	while (StarPos < TotalFileSize)
	{
		FSubHttpTask HttpTask;
		HttpTask.TaskID = TaskID;
		HttpTask.StarPos = StarPos;
		HttpTask.URL = Url;

		if (TotalFileSize > (StarPos + TaskSize))
		{
			HttpTask.Size = TaskSize;
			HttpTask.Range = FString::Printf(TEXT("bytes=%d-%d"), HttpTask.StarPos, StarPos + HttpTask.Size-1);
		}
		else
		{
			HttpTask.Size = TotalFileSize - StarPos;
			HttpTask.Range = FString::Printf(TEXT("bytes=%d-"), HttpTask.StarPos);
		}

		check(HttpTask.Size > 0);
		SubTasks.Add(HttpTask);

		++TaskID;
		StarPos += HttpTask.Size;
	}
}

void UAsyncHttpFile::BeginDownload()
{
	CreateSubTask();	
	UpdateTask();
}

void UAsyncHttpFile::UpdateTask()
{
	int32 NumWaitResponse = 0;

	int32 i = 0;
	for (i = 0; i < SubTasks.Num(); ++i)
	{
		FSubHttpTask &Task = SubTasks[i];
		if (Task.bWaitResponse)
		{
			NumWaitResponse++;
		}
	}

	if (NumWaitResponse<MaxTask)
	{
		for (i = 0; i < SubTasks.Num(); ++i)
		{
			FSubHttpTask &Task = SubTasks[i];
			if (!Task.bFinished && !Task.bWaitResponse)
			{
				NumWaitResponse++;
				StartSubHttpTask(Task);
			}

			if (NumWaitResponse >= MaxTask)
			{
				break;
			}
		}
	}
}

void UAsyncHttpFile::CheckDeadTask(double RealTime)
{
	for (int32 i = 0; i < SubTasks.Num(); ++i)
	{
		FSubHttpTask &Task = SubTasks[i];
		if (!Task.bFinished && Task.bWaitResponse)
		{
			float TimePast = RealTime - Task.RequestTime;
			if (TimePast >= Mgr->ResponseTimeout)
			{
				StartSubHttpTask(Task);
			}
		}
	}
}

void UAsyncHttpFile::StartSubHttpTask(FSubHttpTask &HttpTask)
{
	TSharedPtr<IHttpRequest> Request = HttpTask.CreateRequest();
	HttpTask.bWaitResponse = true;
	HttpTask.RequestTime = FApp::GetCurrentTime();
	Request->OnProcessRequestComplete().BindUObject(this, &UAsyncHttpFile::HandleDownload, HttpTask.TaskID);
	Request->ProcessRequest();
}

void UAsyncHttpFile::HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 TaskID)
{
	if (SubTasks.IsValidIndex(TaskID))
	{
		FSubHttpTask &Task = SubTasks[TaskID];

		if (HttpResponse.IsValid())
		{
			if (bSucceeded)
			{
				Task.RawData = HttpResponse->GetContent();
				check(Task.RawData.Num() == Task.Size);
				Task.bFinished = true;
				Task.bWaitResponse = false;
				Task.Request = nullptr;
				UpdateProgress();
			}
			else
			{
				Task.Request = nullptr;
				Task.bWaitResponse = false;
			}

			if (State != EHFTaskState::ESuccees)
			{
				UpdateTask();
			}
		}
		else
		{
			Task.Request = nullptr;
			Task.bWaitResponse = false;
			FatalErr("HandleDownload Response is null.");
		}
	}
}

void UAsyncHttpFile::Stop()
{
	if (FileSizeRequest.IsValid())
	{
		FileSizeRequest->CancelRequest();
		FileSizeRequest = nullptr;
	}

	for (int32 i = 0; i < SubTasks.Num(); ++i)
	{
		FSubHttpTask &Task = SubTasks[i];
		if (Task.bWaitResponse)
		{
			Task.Stop();
		}
	}
	SubTasks.Empty();

	if (OnCancel.IsBound())
	{
		OnCancel.Broadcast(this, GetDstFilePath(), 0);
	}
}

void UAsyncHttpFile::UpdateProgress()
{
	int32 NumFinished = 0;
	
	for (int32 i = 0; i < SubTasks.Num(); ++i)
	{
		FSubHttpTask &Task = SubTasks[i];
		if (Task.bFinished)
		{
			NumFinished++;
		}
	}

	Progress = float(NumFinished) / SubTasks.Num();

	if (OnProgress.IsBound())
	{
		OnProgress.Broadcast(this, GetDstFilePath(), Progress);
	}

	if (NumFinished == SubTasks.Num())
	{
		OnFinished();
	}
}

void UAsyncHttpFile::OnFinished()
{
	State = EHFTaskState::ESuccees;

	SaveToFile();

	if (OnSuccess.IsBound())
	{
		OnSuccess.Broadcast(this, GetDstFilePath(), Progress);
		Mgr->NotifyFinised(this);
	}
}

void UAsyncHttpFile::FatalErr(const FString &ErrString)
{
	Progress = 0;
	State = EHFTaskState::EFailed;
	OnFail.Broadcast(this, "URL and FileName must set", Progress);
	Mgr->NotifyFailed(this, true);
}

void  UAsyncHttpFile::NotifyFailed()
{
	State = EHFTaskState::EFailed;

	if (OnFail.IsBound())
	{
		OnFail.Broadcast(this, GetDstFilePath(), Progress);
	}
}

void  UAsyncHttpFile::NotifyWait()
{
	State = EHFTaskState::EWaitTry;

	if (OnWait.IsBound())
	{
		OnWait.Broadcast(this, GetDstFilePath(), Progress);
	}
}

void UAsyncHttpFile::SaveToFile()
{
	FString DstFilePath = GetDstFilePath();

	FArchive *Writer = IFileManager::Get().CreateFileWriter(*DstFilePath);
	if (Writer)
	{
		for (int32 i = 0; i < SubTasks.Num(); ++i)
		{
			FSubHttpTask &Task = SubTasks[i];
			Writer->Serialize(Task.RawData.GetData(), Task.RawData.Num());
		}
		Writer->Close();
	}
}

FString UAsyncHttpFile::GetDstFilePath()
{
	return  FPaths::GameSavedDir() / "Download/Resources/" + FileName;
}

