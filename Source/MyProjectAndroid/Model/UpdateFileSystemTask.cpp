
#include "UpdateFileSystemTask.h"
#include "ResourceMgr.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

FUpdateFileSystemAsyncTask::~FUpdateFileSystemAsyncTask()
{
}

void FUpdateFileSystemAsyncTask::DoWork()
{
	if (ResMgr != NULL)
	{
		TArray<FString> FoundDirectories;
		FoundDirectories.Add(FPaths::ProjectContentDir() + TEXT("_Basic/Materials/BaseSx/"));
		FoundDirectories.Add(FPaths::ProjectContentDir() + TEXT("Geometry/"));
		FoundDirectories.Add(FPaths::ProjectSavedDir() + TEXT("Download/Resources/"));
		FoundDirectories.Add(FPaths::ProjectSavedDir() + TEXT("Resources/Local/"));
		FString MXExternDir;
		if (GConfig->GetString(TEXT("Iray"), TEXT("IrayMXDir"), MXExternDir, GGameIni))
		{
			FoundDirectories.Add(MXExternDir);
		}
		for (int32 dirIndex = 0; dirIndex < FoundDirectories.Num(); ++dirIndex)
		{
			TArray<FString> FoundFiles;
			FString DirName = FoundDirectories[dirIndex];

			if (ResMgr->IsAbandonUpdateFile())
			{
				break;
			}

			for (int32 i = 0; i < EResType::EResUnknown; ++i)
			{
				if (ResMgr->IsAbandonUpdateFile())
				{
					break;
				}
				FString FileFilter = FString::Printf(TEXT("*.%s"), GetResTypeName(EResType(i)));
				IFileManager::Get().FindFilesRecursive(FoundFiles, *DirName, *FileFilter, true, false, false);
			}

			for (int32 i = 0; i < FoundFiles.Num(); ++i)
			{
				if (ResMgr->IsAbandonUpdateFile())
				{
					break;
				}
				FString Filename = FoundFiles[i];
				ResMgr->Preload(Filename, false, false);
			}
		}

		if (!ResMgr->IsAbandonUpdateFile())
		{
			ResMgr->FinishUpdateFile();
		}
	}
}

UResource * FUpdateFileSystemAsyncTask::LoadByID(UResourceMgr *ResMgr, const FString &ResID)
{
	UResource *FoundRes = NULL;
	if (ResID.IsEmpty())
	{
		return FoundRes;
	}

	TArray<FString> FoundDirectories;
	FoundDirectories.Add(FPaths::ProjectContentDir() + TEXT("_Basic/Materials/BaseSx/"));
	FoundDirectories.Add(FPaths::ProjectContentDir() + TEXT("Geometry/"));
	FoundDirectories.Add(FPaths::ProjectSavedDir() + TEXT("Download/Resources/"));
	FoundDirectories.Add(FPaths::ProjectSavedDir() + TEXT("Resources/Local/"));
	
	FString MXExternDir;
	if (GConfig->GetString(TEXT("Iray"), TEXT("IrayMXDir"), MXExternDir, GGameIni))
	{
		FoundDirectories.Add(MXExternDir);
	}

	for (int32 dirIndex = 0; dirIndex < FoundDirectories.Num(); ++dirIndex)
	{
		TArray<FString> FoundFiles;
		FString DirName = FoundDirectories[dirIndex];

		for (int32 i = 0; i < EResType::EResUnknown; ++i)
		{
			FString FileFilter = FString::Printf(TEXT("*_%s.%s"), *ResID, GetResTypeName(EResType(i)));
			IFileManager::Get().FindFilesRecursive(FoundFiles, *DirName, *FileFilter, true, false, false);
		}

		if (FoundFiles.Num() > 0)
		{
			FString Filename = FoundFiles[0];
			FoundRes = ResMgr->Preload(Filename, true, false);
			if (!FoundRes)
			{
				UE_LOG(LogResMgr, Error, TEXT("Preload file :%s failed!"), *FoundFiles[0]);
			}
			return FoundRes;
		}
	}

	if (!FoundRes)
	{
		UE_LOG(LogResMgr, Error, TEXT("Preload file :%s not found!"), *ResID);
	}

	return FoundRes;
}

UResource *FUpdateFileSystemAsyncTask::LoadByFilename(UResourceMgr *ResMgr, const FString &Filename, bool bNeedHeader)
{
	UResource *Resource = NULL;
	if (IFileManager::Get().FileExists(*Filename))
	{
		Resource = ResMgr->Preload(Filename, bNeedHeader, false);
	}
	else
	{
		UE_LOG(LogResMgr, Error, TEXT("Preload file :%s failed!"), *Filename);
	}
	return Resource;
}


