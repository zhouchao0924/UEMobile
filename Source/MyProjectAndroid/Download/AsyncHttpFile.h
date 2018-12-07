// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncHttpFile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHttpDownloadDelegate, class UAsyncHttpFile *, HttpFile, const FString &, DestinationPath, float, DownloadPercent);

class FHttpDownloadMgr;

UENUM(BlueprintType)
enum class EHFTaskState : uint8
{
	EPending,
	EDownloading,
	ESuccees,
	EWaitTry,
	EFailed,
};

USTRUCT(BlueprintType)
struct FSubHttpTask
{
	GENERATED_BODY()
	FSubHttpTask();
	int32					 TaskID;
	FString					 URL;
	FString					 Range;
	bool					 bFinished;
	bool					 bWaitResponse;
	int32					 StarPos;
	int32					 Size;
	TArray<uint8>			 RawData;
	double					 RequestTime;
	TSharedPtr<IHttpRequest> Request;
	TSharedPtr<IHttpRequest> CreateRequest();
	void Stop();
};

UCLASS(BlueprintType)
class UAsyncHttpFile : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HttpDownloader", meta = (WorldContext = "WorldContextObject"))
	static UAsyncHttpFile *LoadHttpFile(UObject *WorldContextObject, const FString &url, const FString &FileName, const FString &Directory);
	
	UFUNCTION(BlueprintCallable, Category = "HttpDownloader")
	static void SetMaxParallel(int32 NumParallel);

	UFUNCTION(BlueprintCallable, Category = "HttpDownloader")
	static void SetMaxTryCount(int32 TryCount);

	UFUNCTION(BlueprintCallable, Category = "HttpDownloader")
	static void SetURLRequestKbSize(int32 NumKB);

	UFUNCTION(BlueprintCallable, Category = "HttpDownloader")
	static void SetTimeOut(float Timeout);

	UFUNCTION(BlueprintCallable, Category = "HttpDownloader")
	void Stop();

	void StartTask();
	void CheckDeadTask(double RealTime);
	EHFTaskState GetState() { return State; }
	int32 GetTryCount() { return TryCount; }
	void  NotifyFailed();
	void  NotifyWait();
protected:
	void FatalErr(const FString &ErrString);
	void RequestFileSize();
	void HandleResponseFileSize(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	void HandleDownload(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded, int32 Id);
	void BeginDownload();
	void CreateSubTask();
	void UpdateTask();
	void StartSubHttpTask(FSubHttpTask &HttpTask);
	void UpdateProgress();
	void OnFinished();
	void SaveToFile();
	FString GetDstFilePath();
public:
	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnWait;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnFail;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnProgress;

	UPROPERTY(BlueprintAssignable)
	FHttpDownloadDelegate OnCancel;

	friend class FHttpDownloadMgr;
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	EHFTaskState			State;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					TryCount;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					Url;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					FileName;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	FString					Directory;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	float					Progress;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					CurFileSize;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					TotalFileSize;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	TArray<FSubHttpTask>	SubTasks;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "HttpDownloader")
	int32					MaxTask;
	double					LastActiveTime;
	FHttpDownloadMgr		*Mgr;
	TSharedPtr<IHttpRequest> FileSizeRequest;
};

