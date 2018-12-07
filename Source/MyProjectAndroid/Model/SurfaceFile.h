// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Resource.h"
#include "ModelCommons.h"
#include "SurfaceFile.generated.h"

#define  SURFACE_HEADERVER	5
#define  SURFACE_BODY_VER	1

UCLASS(BlueprintType)
class USurfaceFile : public UResource
{
	GENERATED_UCLASS_BODY()

	struct FHeader : public FResourceSummary
	{
		TArray<FChunk>	Textures;
		int32 GetHeadVer() override { return SURFACE_HEADERVER; }
		int32 GetCodeVer() override { return SURFACE_BODY_VER; }
		void Serialize(FArchive &Ar) override;
	};

public:
	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	UMaterialInterface *GetUE4Material();

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	int32 GetNumerOfTexureSlot();

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	int32 GetNumberOfScalarValues();

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	float GetScalarValue(int32 iSlot, FString &Name);

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	void SetScalarValue(int32 iSlot, float Value);

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	int32 GetNumberOfVectorValues();

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	FLinearColor GetVectorValue(int32 iSlot, FString &Name);

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	void SetVectorValue(int32 iSlot, const FLinearColor &Value);

	UFUNCTION(BlueprintCallable, Category = "DR|Surface")
	UMaterialInstanceDynamic *GetDynamicMaterialInstance();

	virtual void BeginDestroy() override;
	void Serialize(FArchive &Ar) override;
	void SerializeHeader(FArchive &Ar) override;
	FResourceSummary *GetSummary() override { return &m_Header; }

	virtual EResType GetResType() { return EResSurface; }
	FModelMaterial		   *GetMaterial();
	FModelMaterialUE4	   *GetMaterialUE4();
	
	void SetType(EMaterialType InType);
	void SetSurface(FModelMaterial *Material, TArray<FModelTexture *> &Textures);
	void UpdateParameters(UMaterialInstanceDynamic *Mtrl);
	void CopyOverrideParameters(FModelMaterial *Material);
	void CopyOverrideTextures(FModelMaterial *Material, TArray<FModelTexture *> &Textures);
	UTexture *GetTexture(int32 iSlot);
	FModelTexture *GetTextureByName(const FName &Name);
protected:
	virtual void Unload() override;
	virtual void CheckResource() override;
	void SerizlizeHistory(FArchive &Ar, uint32 HistoryFileVer);
	void Clean();
public:
	FHeader						m_Header;
	uint32						FileVer;
	bool						bCompressed;
	FModelMaterialUE4			*m_MaterialInfo;
	TArray<FModelTexture*>		m_Textures;
	
};

