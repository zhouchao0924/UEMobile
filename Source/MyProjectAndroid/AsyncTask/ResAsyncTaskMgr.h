
#pragma once

#include "HAL/Runnable.h"
#include "Tickable.h"
#include "AsyncWork.h"
#include "ProtocalTask.h"
#include "DownloadTask.h"
#include "CookTask.h"
#include "LoadTask.h"
#include "UploadLoadTask.h"

class FImportAsyncTask;
class FResLoadAsyncTask;
class FUploadAsyncTask;
class FDownloadAsyncTask;
class FCookAsyncTask;

class FDRAsyncTaskManager : FTickableGameObject
{

public:
	static FDRAsyncTaskManager &Get();

	virtual void Tick(float DeltaTime);

	virtual bool IsTickable() const { return true; }

	/** return the stat id to use for this tickable **/
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FDRAsyncTaskManager, STATGROUP_Tickables);
	}

	template<typename TTask>
	void QueueTask(FAsyncTask<TTask> *Task);

	void Clear();
	void ClearDownload();

	template<typename TTask>
	void ClearTasks(TArray<FAsyncTask<TTask> *> &Tasks)
	{
		if (Tasks.Num() > 0)
		{
			FAsyncTask<TTask> *Task = Tasks[0];
			if (Task)
			{
				if (!Task->Cancel())
				{
					Task->EnsureCompletion(false);
				}
			}

			for (int32 i = 0; i < Tasks.Num(); ++i)
			{
				FAsyncTask<TTask> *CurTask = Tasks[i];
				if (CurTask)
				{
					CurTask->GetTask().Clear();
					delete CurTask;
				}
			}
			Tasks.Empty();
		}
	}

	template<typename TTask>
	void TickTasks(TArray<FAsyncTask<TTask> *> &Tasks)
	{
		if (Tasks.Num() > 0)
		{
			FAsyncTask<TTask> *pTask = Tasks[0];

			if (pTask->IsIdle() && !pTask->GetTask().bWorking)
			{
				pTask->GetTask().bWorking = true;
				if (pTask->GetTask().IsNeedAsyncThread())
				{
					pTask->StartBackgroundTask();
				}
				else
				{
					pTask->GetTask().DoWork();
				}
			}
			else
			{
				bool bFinished = false;

				if (pTask->IsDone() && pTask->GetTask().IsNeedAsyncThread())
				{
					bFinished = true;
				}
				else
				{
					if (pTask->IsIdle() && !pTask->GetTask().IsNeedAsyncThread())
					{
						if (pTask->GetTask().IsTaskDone())
						{
							bFinished = true;
						}
					}
				}

				if (bFinished)
				{
					pTask->GetTask().ExecuteDone();
					delete pTask;
					Tasks.RemoveAtSwap(0);
				}
			}
		}
	}

	template<typename TTask>
	void ExecuteTask(FAsyncTask<TTask> *pTask);

protected:
	TArray<FAsyncTask<FResLoadAsyncTask> *>		QueueLoadTasks;
	TArray<FAsyncTask<FUploadAsyncTask> *>		QueueUploadTasks;
	TArray<FAsyncTask<FCookAsyncTask> *>		QueueCookTasks;
	TArray<FAsyncTask<FProtocalTask> *>			ProtocalTasks;
	TArray<FAsyncTask<FDownloadAsyncTask> *>	DownloadTasks;
};

template<> void FDRAsyncTaskManager::QueueTask<FResLoadAsyncTask>(FAsyncTask<FResLoadAsyncTask> *pTask)
{
	QueueLoadTasks.Add(pTask);
}

template<> void FDRAsyncTaskManager::QueueTask<FUploadAsyncTask>(FAsyncTask<FUploadAsyncTask> *pTask)
{
	QueueUploadTasks.Add(pTask);
}

template<> void FDRAsyncTaskManager::QueueTask<FCookAsyncTask>(FAsyncTask<FCookAsyncTask> *pTask)
{
	QueueCookTasks.Add(pTask);
}

template<> void FDRAsyncTaskManager::ExecuteTask<FProtocalTask>(FAsyncTask<FProtocalTask> *pTask)
{
	if (pTask->IsIdle() && !pTask->GetTask().bWorking)
	{
		pTask->GetTask().bWorking = true;

		if (pTask->GetTask().IsNeedAsyncThread())
		{
			pTask->StartBackgroundTask();
		}
		else
		{
			pTask->GetTask().DoWork();
		}

		ProtocalTasks.Add(pTask);
	}
}

template<> void FDRAsyncTaskManager::ExecuteTask<FDownloadAsyncTask>(FAsyncTask<FDownloadAsyncTask> *pTask)
{
	if (pTask->IsIdle() && !pTask->GetTask().bWorking)
	{
		pTask->GetTask().bWorking = true;

		if (pTask->GetTask().IsNeedAsyncThread())
		{
			pTask->StartBackgroundTask();
		}
		else
		{
			pTask->GetTask().DoWork();
		}

		DownloadTasks.Add(pTask);
	}
}


