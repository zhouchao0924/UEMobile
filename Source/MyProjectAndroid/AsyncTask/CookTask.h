
#pragma once

#include "ResourceItem.h"
#include "ResAsyncTask.h"

class UResource;
class UModelFile;
class FCookAsyncTask : public FDRAsyncTask
{
public:
	FCookAsyncTask(UResource *InResource,	const FCookArgs &InArgs);
	void DoWork();
	void ExecuteDone() override;
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FCookAsyncTask, STATGROUP_ThreadPoolAsyncTasks); }
protected:
	void CookModelFile(UModelFile *ModelFile);
protected:
	UResource *Resource;
	FCookArgs  CookArgs;
};

