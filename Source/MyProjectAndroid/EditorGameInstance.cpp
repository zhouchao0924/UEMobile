
#include "EditorGameInstance.h"
#include "FileManagerGeneric.h"
#include "ResourceMgr.h"
#include "Protocol/Protocal.h"
#include "AsyncTask/ResAsyncTaskMgr.h"

UCEditorGameInstance::UCEditorGameInstance(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCEditorGameInstance::Init()
{
	Super::Init();

	Context.ResManager = UResourceMgr::Create(this);

	if (Context.ProtocalClass)
	{
		Context.Protocal = NewObject<UProtocalImpl>(this, Context.ProtocalClass);
		if (Context.Protocal)
		{
			Context.Protocal->MyGame = this;
		}
	}
}

void UCEditorGameInstance::Shutdown()
{
	if (Context.ResManager)
	{
		UResourceMgr::Destroy();
		Context.ResManager = NULL;
	}

	FDRAsyncTaskManager::Get().Clear();	
}














