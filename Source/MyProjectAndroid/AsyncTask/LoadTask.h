
#pragma once

#include "Resource.h"
#include "ResAsyncTask.h"

class UResource;
class FResLoadAsyncTask :public FDRAsyncTask
{
public:
	FResLoadAsyncTask(UResource *Resource);
	void DoWork();
	void ExecuteDone() override;
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FResLoadAsyncTask, STATGROUP_ThreadPoolAsyncTasks);}
protected:
	UResource			*PendingResource;
	TArray<UResource *> PendLoadingResources;
};



