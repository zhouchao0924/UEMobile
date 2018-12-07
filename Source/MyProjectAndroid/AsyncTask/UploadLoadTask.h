
#pragma once

#include "Resource.h"
#include "ResAsyncTask.h"
#include "Protocol/Protocal.h"
#include "Interfaces/IHttpRequest.h"

class UResource;
class FUploadAsyncTask :public FDRAsyncTask
{
public:
	FUploadAsyncTask(UResource *Resource, const FString &InURL);
	FUploadAsyncTask(UResource *Resource, const FString &InURL,int32 adduserid);
	void DoWork();
	bool IsTaskDone() override;
	void ExecuteDone() override;
	void Clear() override;
	bool IsNeedAsyncThread() override { return false; }
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FUploadAsyncTask, STATGROUP_ThreadPoolAsyncTasks);}
protected:
	void OnResponseReceived(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
protected:
	FString					 URL;
	TSharedPtr<IHttpRequest> Request;
	UResource			 	 *PendingResource;
	TArray<UResource *>		 PendLoadingResources;
	TSharedPtr<FJsonObject>	 JsonObj;
	bool					 bUploadFinished;
	bool					 bCommitted;
	bool					 bFailed;
	FProtocalDelegate		 Delegate;
	int32					 AddUserId;
};

