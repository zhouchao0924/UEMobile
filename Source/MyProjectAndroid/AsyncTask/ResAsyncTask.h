
#pragma once

#include "AsyncWork.h"
#include "UObject/GCObject.h"

class FDRAsyncTask : public FNonAbandonableTask, public FGCObject
{
public:
	FDRAsyncTask();
	//Game thread
	virtual bool IsNeedAsyncThread() { return true; }
	virtual bool IsTaskDone() { return true; }
	virtual void ExecuteDone() {} 
	virtual void Clear() {}
	virtual void AddReferencedObjects(FReferenceCollector& Collector) {}
	friend class FDRAsyncTaskManager;
protected:
	bool	bWorking;
};

