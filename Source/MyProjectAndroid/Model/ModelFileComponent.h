
#pragma once

#include "ModelPartComponent.h"
#include "ModelFileComponent.generated.h"

UCLASS(BlueprintType, meta=(BlueprintSpawnableComponent))
class UModelFileComponent : public USceneComponent
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void UpdateModel(UModelFile *Model);
	
	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void UpdateMaterial(int32 MaterialIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void SetMaterialByPart(int32 iPart, UMaterialInterface *Material);

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void SetMaterialByChannel(int32 iMtrlChannel, UMaterialInterface *Material);

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	UModelPartComponent *GetPart(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	int32 GetNumOfParts() { return Parts.Num(); }
	 
	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	UModelFile *GetModel();

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void UpdatePhysics();

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void SetSimulatePhysic(bool bSimulate);

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void SetAffectByShadow(bool bAffectByShadow);

	UFUNCTION(BlueprintCallable, Category = "DR|ModelComponent")
	void SetCastShadow(bool bCastShadow);

	FBox GetWorldBounds();
	void SetModelAlignType(ECenterAdjustType InAdjustType);
	void DestroyComponent(bool bPromoteChildren  = false ) override;
	FTransform GetOffset(UModelFile *Model, ECenterAdjustType AlignType);
	FTransform GetRelativeTransform(UModelFile *Model);
	void AddClipPlane(const FPlane &InPlane);
public:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|ModelComponent")
	TSubclassOf<UStandardMaterialCollection>  StandardMaterials;
protected:
	UPROPERTY(Transient)
	FString				ModelResID;
	UPROPERTY(Transient)
	FBox				LocalBounds;
	UPROPERTY(Transient, BlueprintReadOnly)
	ECenterAdjustType	AdjustType;
	TArray<FPlane>		ClipPlanes;
public:
	UPROPERTY(Transient)
	TArray<UModelPartComponent*> Parts;
};

