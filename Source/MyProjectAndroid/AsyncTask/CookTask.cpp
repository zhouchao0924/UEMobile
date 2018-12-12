
#include "CookTask.h"
#include "HttpModule.h"
#include "ModelFile.h"
#include "Interfaces/IHttpRequest.h"
//#include "ConvexHullPhysicBody.h"

//////////////////////////////////////////////////////////////////////////
FCookAsyncTask::FCookAsyncTask(UResource *InResource, const FCookArgs &InArgs)
	:Resource(InResource)
	,CookArgs(InArgs)
{
}

void FCookAsyncTask::DoWork()
{
	bool bForceLoaded = false;

	if (Resource && Resource->IsNeedLoad())
	{
		bForceLoaded = true;
		Resource->ForceLoad();
	}

	UModelFile *ModelFile = Cast<UModelFile>(Resource);
	if (ModelFile)
	{
		CookModelFile(ModelFile);
	}

	if (bForceLoaded)
	{
		Resource->Unload();
	}
}

void FCookAsyncTask::CookModelFile(UModelFile *ModelFile)
{
	if (ModelFile)
	{
		if (CookArgs.bCheckCollision && !ModelFile->IsCookedPhysic())
		{
			//GenerateModelFileConvexHull(ModelFile, 0.8f, 16, false);
		}
		
		if (CookArgs.bAdjustCenter)
		{
			ModelFile->SetCenterType(ECenterAdjustType::BottomCenter);
		}

		ModelFile->MarkDirty();
	}
}

void FCookAsyncTask::ExecuteDone()
{
	if (Resource)
	{
		Resource->OnNotify.Broadcast(Resource, EResNotify::ECooked);
	}
}

void FCookAsyncTask::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Resource);
}



