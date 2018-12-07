
#pragma once

#include "Resource.h"
#include "ResAsyncTask.h"
#include "Protocol/Protocal.h"
#include "Interfaces/IHttpRequest.h"

class UResource;
class FProtocalTask :public FDRAsyncTask
{
public:
	FProtocalTask(UResource *Resource, UVaRestJsonObject *JsonObj, FProtocalDelegate &InDelegate);
	void DoWork();
	void Clear() override;
	bool IsTaskDone() override;
	void ExecuteDone() override;
	bool IsNeedAsyncThread() override { return false; }
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FProtocalTask, STATGROUP_ThreadPoolAsyncTasks);}
protected:
	void OnResponseReceived(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
protected:
	UResource				 *Resource;
	bool					 bFailed;
	bool					 bSuccessed;
	FString					 JsonStr;
	TSharedPtr<IHttpRequest> Request;
	FProtocalDelegate		 Delegate;
};

