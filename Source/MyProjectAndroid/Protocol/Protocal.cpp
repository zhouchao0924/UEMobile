
#include "Protocal.h"
#include "AsyncTask/ResAsyncTaskMgr.h"
#include "EditorGameInstance.h"

UProtocalImpl::UProtocalImpl(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	,MyGame(NULL)
{
}

UProtocalImpl *UProtocalImpl::GetProtocal(UObject *Obj)
{
	UWorld *MyWorld = Obj ? Obj->GetWorld() : NULL;
	UCEditorGameInstance *MyGame = Cast<UCEditorGameInstance>(MyWorld ? MyWorld->GetWorld() : NULL);
	if (MyGame)
	{
		return MyGame->Context.Protocal;
	}
	return NULL;
}

void UProtocalImpl::CallProtocal(UVaRestJsonObject * JsonObj, FProtocalDelegate Delegate)
{
	FDRAsyncTaskManager::Get().ExecuteTask<FProtocalTask>(new FAsyncTask<FProtocalTask>(nullptr, JsonObj, Delegate));
}

