// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ResourceItem.h"
#include "ModelCommons.h"
#include "Resource.h"
#include "SurfaceFile.h"
#include "ModelMaterial.h"
#include "ModelFile.generated.h"

#define  MODELFILE_HEADER_VER	3
#define  MODELFILE_BODY_VER_1	1	// -> 2  collision
#define  MODELFILE_BODY_VER_2	2   // -> 3  UCX  
#define  MODELFILE_BODY_VER_3	3	// -> 4	 Offset
#define  MODELFILE_BODY_VER_4	4	// -> 5  Scale
#define  MODELFILE_BODY_VER_5	5	// -> 6  Depth Width Height replace scale3d
#define  MODELFILE_BODY_VER		6

USTRUCT(BlueprintType)
struct FMaterialSlot
{
	GENERATED_BODY()
	FMaterialSlot();

	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32 MaterialIndex;

	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32 iModel;

	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32 iSection;
};

UCLASS(BlueprintType)
class UModelFile : public UResource
{
	GENERATED_UCLASS_BODY()

	struct FHeader : public FResourceSummary
	{
		TArray<FChunk>	Models;
		TArray<FChunk>	Materials;
		TArray<FChunk>	Textures;
		void Serialize(FArchive &Ar);
		int32 GetHeadVer() override { return MODELFILE_HEADER_VER; }
		int32 GetCodeVer() override { return MODELFILE_BODY_VER; }
	};

	friend class UModelPartComponent;
	friend class UModelFileComponent;
	friend class UResourceMgr;

public:	
	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void Compress();
	
	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void GetSurfaces(TArray<USurfaceFile*>& Surfaces);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	int32 GetNumberOfMaterial();

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void GetMaterialInfo(int32 MaterialIndex, TArray<FMaterialSlot> &materialSlots);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	int32 GetNumerOfTexureSlot(int32 iMaterial);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void GetPreviewCamera(FVector &OutEyeLoc, FVector &OutFocusLoc);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetPreviewCamera(const FVector &EyeLoc, const FVector &FocusLoc);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	bool SetMaterialSlotByUE4Material(int32 MaterialIndex, UMaterialInterface *InMaterial);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	bool SetMaterialSlotBySurface(int32 MaterialIndex, USurfaceFile *Surface);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	EMaterialType GetMaterialType(int32 MaterialIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void ResetMaterial(int32 MaterialIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	int32 GetNumberOfScalarValues(int32 MaterialIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	float GetScalarValue(int32 MaterialIndex, int32 iSlot, FString &Name);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetScalarValue(int32 MaterialIndex, int32 iSlot, float Value);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	int32 GetNumberOfVectorValues(int32 MaterialIndex);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	FLinearColor GetVectorValue(int32 MaterialIndex, int32 iSlot, FString &Name);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetVectorValue(int32 MaterialIndex, int32 iSlot, const FLinearColor &Value);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
    UMaterialInstanceDynamic *GetUE4Material(int32 MaterialIndex);

	void BeginDestroy() override;
	const TArray<FString> * GetDepends() override;

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void ChangeHand();

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetCenterType(ECenterAdjustType InCenterType);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetOffset(const FVector &InOffset);
	
	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	void SetSizeInMM(int32 Depth, int32 Width, int32 Height);

	UFUNCTION(BlueprintCallable, Category = "DR|Model")
	FVector GetOffsetByCenterType(ECenterAdjustType InCenterType);

public:
	FModel *NewModel();
	FModelTexture *NewTexture();
	FModelMaterialUE4 *NewMaterial();
	FModelTexture *GetTextureBySlotType(int32 iMaterial, ETexSlot slotType);
	FModelTexture *GetTextureByName(int32 iMaterial, const FName &Name);
	FModelTexture *GetTextureByName(FModelMaterial *modelMaterial, const FName &Name);
	void Serialize(FArchive &Ar) override;
	virtual EResType GetResType() { return EResModel; }
	FResourceSummary *GetSummary() override { return &m_Header; }
	FModelMaterial *GetMaterial(int32 MaterialIndex);
	void CaclScale3D();
	void ResetSize();
	void OnPostLoad() override;
	bool IsCookedPhysic();
	virtual void Unload() override;
	TArray<FModel *> &GetSubModels() { return m_Models; }
	FBox GetLocalBounds();
	void CloneModel(UModelFile *OtherModel);
	void RoatModel_Z90();
protected:
	USurfaceFile *ExtractSurface(int32 iSurface, const FSurfaceInfo &ChineseName, const FSurfaceInfo &EnglishName);
	void GetMaterialRefTextures(FModelMaterial *Material, const TArray<FModelTexture*> &Texs, TArray<FModelTexture*> &RefTexs);
	void SerizlizeHistory(FArchive &Ar, uint32 HistoryFileVer);
	void CheckResource() override;
	void ClearResource();
	void UpdateDepends();
	void Clean() override;
	friend void GenerateModelFileConvexHull(UModelFile *ModelFile, float InAccuracy, int32 InMaxHullVerts);
public:
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	ECenterAdjustType					CenterType;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	FVector								Offset;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Model")
	FVector								Scale3D;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	FBox								LocalBounds;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	FVector								EyeLoc;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	FVector								FocusLoc;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	float								EyeDistance;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32								DepthInMM;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32								WidthInMM;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Model")
	int32								HeightInMM;
protected:
	FHeader					m_Header;
	TArray<FModel*>			m_Models;
	TArray<FModelMaterial*>	m_Materials;
	TArray<FModelTexture*>	m_Textures;
};

