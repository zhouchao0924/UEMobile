// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "FileUtls.h"

bool UFileUtils::MoveFileTo(FString To, FString From)
{
	return IFileManager::Get().Move(*To, *From);
}
bool UFileUtils::DeleteFile(FString FilePath)
{
	return IFileManager::Get().Delete(*FilePath);
}
