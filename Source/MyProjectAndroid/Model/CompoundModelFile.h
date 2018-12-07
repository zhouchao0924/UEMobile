// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ResourceItem.h"
#include "ModelCommons.h"
#include "CompoundModelFile.generated.h"

#define COMPOUND_MODELFILE_HEADER_VER	RESOURCE_HEADERVER
#define COMPOUND_MODELFILE_BODY_VER		1

UENUM(BlueprintType)
enum class ECompPosAnchorType : uint8
{
	EPositive,
	ENegative,
	EScale,
};

class UModelFile;

USTRUCT(BlueprintType)
struct FMXComponentSlot 
{
	GENERATED_BODY()

	FMXComponentSlot();
	FMXComponentSlot(int32 InModelIndex, int32 InBoneIndex, const FTransform &InRelativeTransform);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="DR|Model")
	int32							RefModelId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	int32							AttachedBoneIndex;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	bool							bAchoredPositionOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	ECompPosAnchorType				AnchorTypeX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	ECompPosAnchorType				AnchorTypeY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	ECompPosAnchorType				AnchorTypeZ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	bool							ScalableX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	bool							ScalableY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	bool							ScalableZ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	TArray<int32>					ScaleAnchorPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	FTransform						ParentSpaceTransform;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Model")
	class UModelFile				*LoadedModel;

	void Serialize(FArchive &Ar, int32 BodyVersion);
};

USTRUCT(BlueprintType)
struct FLayoutBoneNode
{
	GENERATED_BODY()

	FLayoutBoneNode();
	FLayoutBoneNode(const FString &InName, int32 InParentIndex, const FTransform &InTransform);

	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	FString			Name;

	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32			ParentIndex;
	
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	FTransform		ParentSpaceTransform;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Model")
	int32			AttachedComponentIndex;

	void Serialize(FArchive &Ar, int32 BodyVersion);
};

UCLASS(BlueprintType)
class UCompoundModelFile : public UResource
{
	GENERATED_UCLASS_BODY()
public:
	struct FHeader : public FResourceSummary
	{
		void Serialize(FArchive &Ar) override;
		int32 GetHeadVer() override { return COMPOUND_MODELFILE_HEADER_VER; }
		int32 GetCodeVer() override { return COMPOUND_MODELFILE_BODY_VER; }
	};

	void Serialize(FArchive &Ar) override;
	FResourceSummary *GetSummary() override { return &m_Header; }

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	int32 GetNumerOfModels();

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	UModelFile *GetModelFile(int32 ModelIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	UModelFile *GetModelByComponentSlot(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	bool IsScalableX(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	bool IsScalableY(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	bool IsScalableZ(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	bool IsAnchorPositionOnly(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetScalableX(int32 SlotIndex, bool bScalable);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetScalableY(int32 SlotIndex, bool bScalable);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetScalableZ(int32 SlotIndex, bool bScalable);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	ECompPosAnchorType GetPosAnchorTypeX(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetPosAnchorTypeX(int32 SlotIndex, ECompPosAnchorType InAxisType);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	ECompPosAnchorType GetPosAnchorTypeY(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetPosAnchorTypeY(int32 SlotIndex, ECompPosAnchorType InAxisType);

	UFUNCTION(BlueprintPure, Category = "DR|Model")
	ECompPosAnchorType GetPosAnchorTypeZ(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetPosAnchorTypeZ(int32 SlotIndex, ECompPosAnchorType InAxisType);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetAnchorPositionOnly(int32 SlotIndex, bool bAnchorPositionOnly);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetSlotModel(int32 SlotIndex, UModelFile *InModelFile);

	//UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	EResType GetResType() override { return EResCompoundModel; }

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	int32 GetAttachedComponentSlotIndex(int32 AttachedBoneIndex);

	void OnPostLoad() override;
	void AddModel(UModelFile *Model, const FString &ParentBoneName, const FTransform &RelativeTransform);
	void AddBone(const FString &Name, int32 ParentIndex, const FTransform &RelativeTransform);
	int32 FindBone(const FString &Name);
	FTransform GetBoneTransform(int32 BoneIndex);
	void UpdateBoneTransforms();
	int32 GetParentBone(int32 BoneIndex);
	void UpdateModelComponentTrasforms(TArray<FTransform> &OutTransforms, const FTransform &ComponentWorldTransform, const TArray<class UModelFileComponent *> &ModelComponents);
protected:
	void UpdateBounds();
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	TArray<FLayoutBoneNode>			Bones;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	TArray<FMXComponentSlot>		MXComponents;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Model")
	FBox							LocalBounds;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Model")
	TArray<UModelFile*>				Models;
protected:
	UPROPERTY(Transient)
	TArray<FTransform>		BoneTransforms;
	FHeader					m_Header;
};



