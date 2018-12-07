// Fill out your copyright notice in the Description page of Project Settings.
// material node
#pragma once
#include "MaterialNode.generated.h"

// wall mat, area mat, tjx mat, bsp mat
USTRUCT(Blueprintable)
struct FMaterialNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ResID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ModelID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Url;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 RoomClassID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 CraftID;

	bool operator ==(const FMaterialNode& OtherMaterial)
	{
		return (ResID == OtherMaterial.ResID) && (ModelID == OtherMaterial.ModelID);
	}
};

// wall mat, area mat, tjx mat, bsp mat
USTRUCT(Blueprintable)
struct FSavedMaterialNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Path;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsProjectMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString PakPath;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString MaterialDatas;
};
