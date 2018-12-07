// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "FileUtls.generated.h"

UCLASS(BlueprintType)
class UFileUtils :public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "FileUtils")
	static bool MoveFileTo(FString To, FString From);

	UFUNCTION(BlueprintCallable, Category = "FileUtils")
	static bool DeleteFile(FString FilePath);
};