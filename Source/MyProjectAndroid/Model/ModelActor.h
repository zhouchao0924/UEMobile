// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Resource.h"
#include "Model/ModelFileComponent.h"
#include "ModelActor.generated.h"

UCLASS(BlueprintType)
class AModelActor :public AActor
{
	GENERATED_UCLASS_BODY()
public:
	void OnConstruction(const FTransform& Transform) override;
	UModelFile *GetModelFile();
	FTransform GetModelTransform();
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString					ResID;
	UPROPERTY(Transient)
	UModelFileComponent		*ModelFileComponent;
};


