
#pragma once

#include "ModelTexture.h"

UENUM(BlueprintType)
enum EMaterialType
{
	EMaterialRef,
	EMaterialUE4,
	EMaterialNone,
};

struct FTexSlotInfo
{
	FTexSlotInfo() { iSlot = INDEX_NONE; iTex = INDEX_NONE; TexParamName = NAME_None; }
	FTexSlotInfo(int32 InSlot, int32 InTex)
		:iSlot(InSlot)
		,iTex(InTex)
		,TexValue(NULL)
	{
	}
	int32		iSlot;
	int32		iTex;
	ETexSlot	slotType;
	FName		TexParamName;
	UTexture	*TexValue;
};

struct FScalarSlotInfo
{
	FScalarSlotInfo() { iSlot = INDEX_NONE; FloatValue = 0; ParamName = NAME_None; }
	int32		 iSlot;
	float		 FloatValue;
	FName		 ParamName;
};

struct FVectorSlotInfo
{
	FVectorSlotInfo() { iSlot = INDEX_NONE; LinearValue = FLinearColor::Black; ParamName = NAME_None; }
	int32		 iSlot;
	FLinearColor LinearValue;
	FName		 ParamName;
};

class FSurfaceParameter
{
public:
	FName	ParamName;
	virtual void Serialize(FArchive &Ar, uint32 Ver);
};

class FSurfaceParameterFloat : public FSurfaceParameter
{
public:
	float Value;
	void Serialize(FArchive &Ar, uint32 Ver) override;
};

class FSurfaceParameterVector : public FSurfaceParameter
{
public:
	FLinearColor Value;
	void Serialize(FArchive &Ar, uint32 Ver) override;
};

class FSurfaceParameterTexture : public FSurfaceParameter
{
public:
	int32 Value;
	void Serialize(FArchive &Ar, uint32 Ver) override;
};

class UStandardMaterialCollection;

class FModelMaterial
{
public:
	virtual FString GetName(UStandardMaterialCollection *MtrlCollection) { return TEXT(""); }
	virtual EMaterialType GetType() { return EMaterialNone; }
	virtual void GetRefTextures(TArray<FTexSlotInfo> &RefTextures, bool bExcludeDeprecated) {}
	virtual void GetRefValues(TArray<FScalarSlotInfo> &RefValues, bool bExcludeDeprecated){}
	virtual void GetRefValues(TArray<FVectorSlotInfo> &RefValues, bool bExcludeDeprecated) {}
	virtual void RemapTexture(const TArray<int32> &Remap);
	virtual void Serialize(FArchive &Ar, uint32 Ver);
	virtual UMaterialInterface *GetUE4Material() { return NULL; }
	static FModelMaterial *CreateMaterial(EMaterialType type);
	int32 FindTextureIndex(const FName &Name);
	int32 FindFloatIndex(const FName &Name);
	int32 FindVectorIndex(const FName &Name);
	int32 GetTextureValue(const FName &Name);
	float GetFloatValue(const FName &Name);
	FLinearColor GetVectorValue(const FName &Name);
	int32 SetTextureValue(const FName &Name, int32 Value);
	int32 SetFloatValue(const FName &Name, float Value);
	int32 SetVectorValue(const FName &Name, const FLinearColor &Value);
	virtual void UpdateParameters(UMaterialInstanceDynamic *Mtrl, TArray<FModelTexture *> &Textures);
	virtual void CopyOverrideParametersTo(FModelMaterial *Material);
	bool IsValidMaterialParameter(FGuid &GUID, FName &Name);
public:
	TArray<FSurfaceParameterFloat>		OverrideFloatParameters;
	TArray<FSurfaceParameterVector>		OverrideVectorParameters;	
	TArray<FSurfaceParameterTexture>	OverrideTextureParameters;
};

class FModelMaterialUE4 :public FModelMaterial
{
public:
	FString Name;
	FString UE4Path;
	virtual FString GetName(UStandardMaterialCollection *MtrlCollection) override;
	virtual EMaterialType GetType() { return EMaterialUE4; }
	virtual void Serialize(FArchive &Ar, uint32 Ver);
	void GetRefTextures(TArray<FTexSlotInfo> &RefTextures, bool bExcludeDeprecated) override;
	void GetRefValues(TArray<FScalarSlotInfo> &RefValues, bool bExcludeDeprecated) override;
	void GetRefValues(TArray<FVectorSlotInfo> &RefValues, bool bExcludeDeprecated) override;
	UMaterialInterface *GetUE4Material() override;
};

class FModelMaterialRef : public FModelMaterial
{
public:
	FString BaseSkuid;
	FWeakObjectPtr RefSurface;
	virtual FString GetName(UStandardMaterialCollection *MtrlCollection) override;
	void Serialize(FArchive &Ar, uint32 Ver) override;
	EMaterialType GetType() override { return EMaterialRef; }
	void GetRefTextures(TArray<FTexSlotInfo> &RefTextures, bool bExcludeDeprecated) override;
	void GetRefValues(TArray<FScalarSlotInfo> &RefValues, bool bExcludeDeprecated) override;
	void GetRefValues(TArray<FVectorSlotInfo> &RefValues, bool bExcludeDeprecated) override;
	UMaterialInterface *GetUE4Material() override;
	void UpdateParameters(UMaterialInstanceDynamic *Mtrl, TArray<FModelTexture *> &Textures) override;
	void CopyOverrideParametersTo(FModelMaterial *Material) override;
};




