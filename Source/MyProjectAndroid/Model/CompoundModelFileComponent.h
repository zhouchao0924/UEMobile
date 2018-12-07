// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CompoundModelFile.h"
#include "CompoundModelFileComponent.generated.h"

UCLASS(BlueprintType,meta=(BlueprintSpawnableComponent))
class UCompoundModelFileComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetCompoundModelFile(UCompoundModelFile *InModelFile);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	int32 GetNumberOfBones();

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	FTransform GetBoneTransform(int32 BoneIndex);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	int32 GetNumberOfModels();
	
	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	UModelFileComponent *GetModelFileComponent(int32 CompIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void UpdateModelComponentsTransform();

	FLayoutBoneNode *GetBone(int32 BoneIndex);
	FBox GetWorldBoundBox() { return WorldBounds; }
protected:
	void DestroyComponent(bool bPromoteChildren /* = false */) override;
	void DestroyChildModels();
	void UpdateWorldBounds();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	UCompoundModelFile				*CompoundModelFile;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	TArray<UModelFileComponent *>	ModelComponents;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Model")
	FBox							WorldBounds;
};


