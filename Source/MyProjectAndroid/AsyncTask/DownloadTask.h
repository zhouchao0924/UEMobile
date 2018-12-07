
#pragma once

#include "Resource.h"
#include "ResourceItem.h"
#include "ResAsyncTask.h"
#include "DownloadTask.generated.h"

class UResource;
class UAsyncHttpFile;
class UDownloadCallback;

class FDownloadAsyncTask :public FDRAsyncTask
{
public:
	FDownloadAsyncTask(UResourceMgr *InResMgr, EResType ResType, const FString &ResID, const FString &InURL, const FString &InFilename, const TArray<FDownloadingRes> &DependsResources, FOnDownloadTaskCompeleted InDelegate, int32 SvrVersion = 0);
	void Stop();
	void DoWork();
	bool IsTaskDone() override;
	void ExecuteDone() override;
	virtual bool IsNeedAsyncThread() { return false; }
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FDownloadAsyncTask, STATGROUP_ThreadPoolAsyncTasks);}
	void OnLoadCompleted(bool bSuccess, UResource *Resource);
	void NotifyHttpDownloadSuccess(UAsyncHttpFile *HttpFile, const FString &Filename);
	void NotifyHttpDownloadFailed(UAsyncHttpFile *HttpFile);
	void NotifyHttpProgress(UAsyncHttpFile *HttpFile, float Progress);
protected:
	EResType					ResType;
	FString						ResID;
	FString						URL;
	FOnDownloadTaskCompeleted	Delegate;
	UResourceMgr				*ResMgr;
	UDownloadCallback			*Callback;
	TArray<FDownloadingRes>		DownloadRes;
	int32						NumLoaded;
	int32						NumFailed;
	int32						SrvVersion;
	FThreadSafeCounter			StopCouter;
	FCriticalSection			MutexDownload;
};

UCLASS()
class UDownloadCallback : public UObject
{
	GENERATED_BODY()
public:
	UDownloadCallback() { Task = NULL; }
	
	UFUNCTION()
	void OnResStatedChanged(UResource * Resource, EResState State, UVaRestJsonObject * DRInfo);
	
	UFUNCTION()
	void OnHttpDownloadSuccess(UAsyncHttpFile *HttpFile, const FString &DestinationPath, float DownloadPercent);
	
	UFUNCTION()
	void OnHttpDownloadFailed(UAsyncHttpFile *HttpFile, const FString &DestinationPath, float DownloadPercent);

	UFUNCTION()
	void OnHttpProgress(UAsyncHttpFile * HttpFile, const FString &DestinationPath, float DownloadPercent);
public:
	FDownloadAsyncTask *Task;
};


