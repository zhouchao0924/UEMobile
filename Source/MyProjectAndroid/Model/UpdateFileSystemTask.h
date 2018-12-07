// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class UResource;
class UResourceMgr;

class FUpdateFileSystemAsyncTask
{
public:
	FUpdateFileSystemAsyncTask(UResourceMgr *InResMgr)
		:ResMgr(InResMgr)
	{
	}
	~FUpdateFileSystemAsyncTask();
	bool CanAbandon(){ return true; }
	void Abandon() {}
	void DoWork();
	FORCEINLINE TStatId GetStatId() const { RETURN_QUICK_DECLARE_CYCLE_STAT(FUpdateFileSystemAsyncTask, STATGROUP_ThreadPoolAsyncTasks); }
	static UResource *LoadByID(UResourceMgr *ResMgr, const FString &ResID);
	static UResource *LoadByFilename(UResourceMgr *ResMgr, const FString &Filename, bool bNeedHeader);
protected:
	UResourceMgr *ResMgr;
};


