// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "StandardMaterialCollection.generated.h"

USTRUCT(BlueprintType)
struct FStdMaterialInfo
{
	GENERATED_BODY();
	FStdMaterialInfo();
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					CategoryName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FStringAssetReference	MaterialAsset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					MDLMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					DiffuseTex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					OpacityTex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					NormalTex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					RoughnessTex;
	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FString>			ExposeParameters;
	UPROPERTY(Transient, BlueprintReadOnly)
	UTexture				*PreviewImage;
	UMaterialInterface	*GetMaterial();
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	UMaterialInterface		*Material;
};

UCLASS(Blueprintable, abstract)
class UStandardMaterialCollection : public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UMaterialInterface  *GetMaterialByName(const FString &Name);
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StandardMaterial")
	TArray<FStdMaterialInfo>	  StdMaterials;
};

UENUM(BlueprintType)
enum class EParameterType : uint8
{
	FloatParameter,
	VectorParameter,
	TexParameter
};

USTRUCT(BlueprintType)
struct FParameterMapping :public FTableRowBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	EParameterType Type;

	UPROPERTY(EditAnywhere)
	FName	UE4Name;
	
	UPROPERTY(EditAnywhere)
	FName	IrayName;
};


