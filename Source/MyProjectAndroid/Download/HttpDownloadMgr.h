// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Stats/Stats.h"
#include "UObject/GCObject.h"

class UAsyncHttpFile;

class FHttpDownloadMgr : public FTickableGameObject, public FGCObject
{
public:
	FHttpDownloadMgr();
	static FHttpDownloadMgr *GetDownloadMgr();

	void SetMaxParallel(int32 NumParallel);
	void SetMaxTryCount(int32 TryCount);
	void SetURLRequestKbSize(int32 NumKB);
	void SetTimeOut(float Timeout);
	

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const { return TStatId();/* RETURN_QUICK_DECLARE_CYCLE_STAT(FHttpDownloadMgr, STATGROUP_Tickables);*/ }

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	UAsyncHttpFile *CreateTask(UObject *WorldContextObject, const FString &Url, const FString &FileName, const FString &Directory);
	void UpdateHttpFiles();
	void ClearMgr();

protected:
	friend class UAsyncHttpFile;
	void NotifyFailed(UAsyncHttpFile *HttpFile, bool bForceFailed);
	void NotifyFinised(UAsyncHttpFile *HttpFile);
public:
	TArray<UAsyncHttpFile *>	DownloadHttpFiles;
	int32						MaxParallel;
	int32						MaxTryCount;
	int32						RequestKBSize;
	float						ResponseTimeout;
	static FHttpDownloadMgr		*DownloadManager;
};

