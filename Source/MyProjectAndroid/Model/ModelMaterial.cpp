
#include "ModelMaterial.h"
#include "ResourceMgr.h"
#include "SurfaceFile.h"
#include "StandardMaterialCollection.h"

//////////////////////////////////////////////////////////////////////////
void FSurfaceParameter::Serialize(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving())
	{
		FString Name = ParamName.ToString();
		Ar << Name;
	}
	else if (Ar.IsLoading())
	{
		FString Name;
		Ar << Name;
		ParamName = *Name;
	}
}

void FSurfaceParameterFloat::Serialize(FArchive &Ar, uint32 Ver)
{
	FSurfaceParameter::Serialize(Ar, Ver);

	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Value;
	}
}

void FSurfaceParameterVector::Serialize(FArchive &Ar, uint32 Ver)
{
	FSurfaceParameter::Serialize(Ar, Ver);

	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Value;
	}
}

void FSurfaceParameterTexture::Serialize(FArchive &Ar, uint32 Ver)
{
	FSurfaceParameter::Serialize(Ar, Ver);

	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Value;
	}
}

//////////////////////////////////////////////////////////////////////////
void FModelMaterial::Serialize(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving())
	{
		int32 NumParas = OverrideFloatParameters.Num();
		Ar << NumParas;
		for (int32 i = 0; i < OverrideFloatParameters.Num(); ++i)
		{
			OverrideFloatParameters[i].Serialize(Ar, Ver);
		}

		NumParas = OverrideVectorParameters.Num();
		Ar << NumParas;
		for (int32 i = 0; i < OverrideVectorParameters.Num(); ++i)
		{
			OverrideVectorParameters[i].Serialize(Ar, Ver);
		}

		NumParas = OverrideTextureParameters.Num();
		Ar << NumParas;
		for (int32 i = 0; i < OverrideTextureParameters.Num(); ++i)
		{
			OverrideTextureParameters[i].Serialize(Ar, Ver);
		}
	}
	else if (Ar.IsLoading())
	{
		int32 NumParas = 0;
		Ar << NumParas;

		OverrideFloatParameters.SetNum(NumParas);
		for (int32 i = 0; i < OverrideFloatParameters.Num(); ++i)
		{
			OverrideFloatParameters[i].Serialize(Ar, Ver);
		}

		Ar << NumParas;
		OverrideVectorParameters.SetNum(NumParas);
		for (int32 i = 0; i < OverrideVectorParameters.Num(); ++i)
		{
			OverrideVectorParameters[i].Serialize(Ar, Ver);
		}

		Ar << NumParas;
		OverrideTextureParameters.SetNum(NumParas);
		for (int32 i = 0; i < OverrideTextureParameters.Num(); ++i)
		{
			OverrideTextureParameters[i].Serialize(Ar, Ver);
		}
	}
}

FModelMaterial *FModelMaterial::CreateMaterial(EMaterialType type)
{
	FModelMaterial *newMaterial = NULL;

	if (type == EMaterialUE4)
	{
		newMaterial = new FModelMaterialUE4();
	}
	else if (type == EMaterialRef)
	{
		newMaterial = new FModelMaterialRef();
	}

	return newMaterial;
}

int32 FModelMaterial::FindTextureIndex(const FName &Name)
{
	int32 FoundIndex = INDEX_NONE;

	for (int32 i = 0; i < OverrideTextureParameters.Num(); ++i)
	{
		FSurfaceParameterTexture &Param = OverrideTextureParameters[i];
		if (Param.ParamName == Name)
		{
			FoundIndex = i;
		}
	}

	return FoundIndex;
}

int32 FModelMaterial::FindFloatIndex(const FName &Name)
{
	int32 FoundIndex = INDEX_NONE;

	for (int32 i = 0; i < OverrideFloatParameters.Num(); ++i)
	{
		FSurfaceParameterFloat &Param = OverrideFloatParameters[i];
		if (Param.ParamName == Name)
		{
			FoundIndex = i;
		}
	}

	return FoundIndex;
}

int32 FModelMaterial::FindVectorIndex(const FName &Name)
{
	int32 FoundIndex = INDEX_NONE;

	for (int32 i = 0; i < OverrideVectorParameters.Num(); ++i)
	{
		FSurfaceParameterVector &Param = OverrideVectorParameters[i];
		if (Param.ParamName == Name)
		{
			FoundIndex = i;
		}
	}

	return FoundIndex;
}

int32 FModelMaterial::GetTextureValue(const FName &Name)
{
	int32 FoundIndex = FindTextureIndex(Name);
	if (FoundIndex != INDEX_NONE)
	{
		return OverrideTextureParameters[FoundIndex].Value;
	}
	return INDEX_NONE;
}

float FModelMaterial::GetFloatValue(const FName &Name)
{
	int32 FoundIndex = FindFloatIndex(Name);
	if (FoundIndex != INDEX_NONE)
	{
		return OverrideFloatParameters[FoundIndex].Value;
	}
	return 0;
}

FLinearColor FModelMaterial::GetVectorValue(const FName &Name)
{
	int32 FoundIndex = FindVectorIndex(Name);
	if (FoundIndex != INDEX_NONE)
	{
		return OverrideVectorParameters[FoundIndex].Value;
	}
	return FLinearColor::Black;
}

int32 FModelMaterial::SetTextureValue(const FName &Name, int32 Value)
{
	int32 FoundIndex = FindTextureIndex(Name);
	if (FoundIndex != INDEX_NONE)
	{
		OverrideTextureParameters[FoundIndex].ParamName = Name;
		OverrideTextureParameters[FoundIndex].Value = Value;
	}
	else
	{
		FoundIndex = OverrideTextureParameters.Num();
		FSurfaceParameterTexture *TexParam = new (OverrideTextureParameters)FSurfaceParameterTexture();
		TexParam->ParamName = Name;
		TexParam->Value = Value;
	}
	return FoundIndex;
}

int32 FModelMaterial::SetFloatValue(const FName &Name, float Value)
{
	int32 FoundIndex = FindFloatIndex(Name);
	if (FoundIndex != INDEX_NONE)
	{
		OverrideFloatParameters[FoundIndex].ParamName = Name;
		OverrideFloatParameters[FoundIndex].Value = Value;
	}
	else
	{
		FoundIndex = OverrideFloatParameters.Num();
		FSurfaceParameterFloat *FloatParam = new (OverrideFloatParameters) FSurfaceParameterFloat();
		FloatParam->ParamName = Name;
		FloatParam->Value = Value;
	}
	return FoundIndex;
}

int32 FModelMaterial::SetVectorValue(const FName &Name, const FLinearColor &Value)
{
	int32 FoundIndex = FindVectorIndex(Name);
	if (FoundIndex != INDEX_NONE)
	{
		OverrideVectorParameters[FoundIndex].ParamName = Name;
		OverrideVectorParameters[FoundIndex].Value = Value;
	}
	else
	{
		FoundIndex = OverrideVectorParameters.Num();
		FSurfaceParameterVector *VecParam = new (OverrideVectorParameters) FSurfaceParameterVector;
		VecParam->ParamName = Name;
		VecParam->Value = Value;
	}
	return FoundIndex;
}

void FModelMaterial::RemapTexture(const TArray<int32> &Remap)
{
	for (int32 i = 0; i < OverrideTextureParameters.Num(); ++i)
	{
		FSurfaceParameterTexture &TexParam = OverrideTextureParameters[i];
		int32 OldIndex = TexParam.Value;
		TexParam.Value = (OldIndex!=INDEX_NONE) ? Remap[OldIndex] : INDEX_NONE;
	}
}

FName GetDeprecatedName(const FName &Name)
{
	FString DeprecatedName = FString::Printf(TEXT("Deprecated_%s"), *(Name.GetPlainNameString()));
	return FName(*DeprecatedName);
}

void FModelMaterial::UpdateParameters(UMaterialInstanceDynamic *Mtrl, TArray<FModelTexture *> &Textures)
{
	UTexture *TexValue = NULL;
	for (int32 i = 0; i < OverrideTextureParameters.Num(); ++i)
	{
		FSurfaceParameterTexture &TexParam = OverrideTextureParameters[i];
		if (Textures.IsValidIndex(TexParam.Value))
		{
			FModelTexture *ModelTex = Textures[TexParam.Value];
			UTexture *Tex = ModelTex? ModelTex->GetTexture():NULL;

			FName ParaName = TexParam.ParamName;
			if (!Mtrl->GetTextureParameterValue(TexParam.ParamName, TexValue))
			{
				ParaName = GetDeprecatedName(TexParam.ParamName);
			}

			Mtrl->SetTextureParameterValue(ParaName, Tex);
		}
	}

	float ScalarValue = 0;
	for (int32 i = 0; i < OverrideFloatParameters.Num(); ++i)
	{
		FSurfaceParameterFloat &FloatParam = OverrideFloatParameters[i];

		FName ParaName = FloatParam.ParamName;
		if (!Mtrl->GetScalarParameterValue(FloatParam.ParamName, ScalarValue))
		{
			ParaName = GetDeprecatedName(FloatParam.ParamName);
		}

		Mtrl->SetScalarParameterValue(ParaName, FloatParam.Value);
	} 

	FLinearColor VectorValue;
	for (int32 i = 0; i < OverrideVectorParameters.Num(); ++i)
	{
		FSurfaceParameterVector &VecParam = OverrideVectorParameters[i];

		FName ParaName = VecParam.ParamName;
		if (!Mtrl->GetVectorParameterValue(VecParam.ParamName, VectorValue))
		{
			ParaName = GetDeprecatedName(VecParam.ParamName);
		}

		Mtrl->SetVectorParameterValue(ParaName, VecParam.Value);
	}
}

void FModelMaterial::CopyOverrideParametersTo(FModelMaterial *Material)
{
	for (int32 i = 0; i < OverrideFloatParameters.Num(); ++i)
	{
		FSurfaceParameterFloat &FloatParam = OverrideFloatParameters[i];
		Material->SetFloatValue(FloatParam.ParamName, FloatParam.Value);
	}

	for (int32 i = 0; i < OverrideVectorParameters.Num(); ++i)
	{
		FSurfaceParameterVector &VecParam = OverrideVectorParameters[i];
		Material->SetVectorValue(VecParam.ParamName, VecParam.Value);
	}
}

bool FModelMaterial::IsValidMaterialParameter(FGuid &GUID, FName &Name)
{
	FString TexName = Name.GetPlainNameString();
	return GUID.IsValid() && TexName.Find(TEXT("Deprecated"))==INDEX_NONE;
}

//////////////////////////////////////////////////////////////////////////
FString FModelMaterialUE4::GetName(UStandardMaterialCollection *MtrlCollection)
{
	if (MtrlCollection)
	{
		for (int32 i = 0; i < MtrlCollection->StdMaterials.Num(); ++i)
		{
			FStdMaterialInfo &Info = MtrlCollection->StdMaterials[i];
			FString Path = Info.MaterialAsset.GetAssetName();
			if (Path == FPaths::GetBaseFilename(UE4Path))
			{
				return Info.DisplayName;
			}
		}
	}
	return FPaths::GetBaseFilename(UE4Path);
}

UMaterialInterface *FModelMaterialUE4::GetUE4Material()
{
	UObject *MaterialObj =  StaticLoadObject(UMaterialInstance::StaticClass(), NULL, *UE4Path);
	if (MaterialObj!=NULL)
	{
		return Cast<UMaterialInstance>(MaterialObj);
	}
	return NULL;
}

void FModelMaterialUE4::GetRefTextures(TArray<FTexSlotInfo> &RefTextures, bool bExcludeDeprecated)
{
	UMaterialInstance *Material = Cast<UMaterialInstance>(GetUE4Material());
	if (Material)
	{
		for (int32 i = 0; i < Material->TextureParameterValues.Num(); ++i)
		{
			FTextureParameterValue &TexParam = Material->TextureParameterValues[i];
			if (!bExcludeDeprecated || IsValidMaterialParameter(TexParam.ExpressionGUID, TexParam.ParameterInfo.Name))
			{
				int32 Value = GetTextureValue(TexParam.ParameterInfo.Name);
				FTexSlotInfo *Slot = new (RefTextures)FTexSlotInfo(i, Value);
				Slot->TexParamName = TexParam.ParameterInfo.Name;
				Slot->TexValue = TexParam.ParameterValue;

				UMaterialInterface *Parent = Material->Parent;
				while (!Slot->TexValue)
				{
					Parent->GetTextureParameterValue(TexParam.ParameterInfo.Name, Slot->TexValue);
					if (!Slot->TexValue)
					{
						UMaterialInstance *ParentInstance = Cast<UMaterialInstance>(Parent);
						Parent = ParentInstance->Parent;
					}
				}
			}
		}
	}
}

void FModelMaterialUE4::GetRefValues(TArray<FScalarSlotInfo> &RefValues, bool bExcludeDeprecated)
{
	UMaterialInstance *Material = Cast<UMaterialInstance>(GetUE4Material());
	if (Material)
	{
		for (int32 i = 0; i < Material->ScalarParameterValues.Num(); ++i)
		{
			FScalarParameterValue &ScaleParam = Material->ScalarParameterValues[i];
			if (!bExcludeDeprecated || IsValidMaterialParameter(ScaleParam.ExpressionGUID, ScaleParam.ParameterInfo.Name))
			{
				int32 iSlot = RefValues.Num();

				FScalarSlotInfo *Slot = new (RefValues) FScalarSlotInfo();
				Slot->iSlot = iSlot;
				Slot->ParamName = ScaleParam.ParameterInfo.Name;

				int32 FoundIndex = FindFloatIndex(ScaleParam.ParameterInfo.Name);
				if (FoundIndex != INDEX_NONE)
				{
					Slot->FloatValue = GetFloatValue(ScaleParam.ParameterInfo.Name);
				}
				else
				{
					Material->GetScalarParameterValue(ScaleParam.ParameterInfo.Name, Slot->FloatValue);
				}
			}
		}
	}
}

void FModelMaterialUE4::GetRefValues(TArray<FVectorSlotInfo> &RefValues, bool bExcludeDeprecated)
{
	UMaterialInstance *Material = Cast<UMaterialInstance>(GetUE4Material());
	if (Material)
	{
		for (int32 i = 0; i < Material->VectorParameterValues.Num(); ++i)
		{
			FVectorParameterValue &VecParam = Material->VectorParameterValues[i];
			if (!bExcludeDeprecated || IsValidMaterialParameter(VecParam.ExpressionGUID, VecParam.ParameterInfo.Name))
			{
				int32 iSlot = RefValues.Num();

				FVectorSlotInfo *Slot = new (RefValues) FVectorSlotInfo();
				Slot->iSlot = iSlot;
				Slot->ParamName = VecParam.ParameterInfo.Name;

				int32 FoundIndex = FindVectorIndex(VecParam.ParameterInfo.Name);
				if (FoundIndex != INDEX_NONE)
				{
					Slot->LinearValue = GetVectorValue(VecParam.ParameterInfo.Name);
				}
				else
				{
					Material->GetVectorParameterValue(VecParam.ParameterInfo.Name, Slot->LinearValue);
				}
			}
		}
	}
}

void FModelMaterialUE4::Serialize(FArchive &Ar, uint32 Ver)
{
	FModelMaterial::Serialize(Ar, Ver);

	if (Ar.IsSaving()|| Ar.IsLoading())
	{
		Ar << UE4Path;
	}
}

//////////////////////////////////////////////////////////////////////////
FString FModelMaterialRef::GetName(UStandardMaterialCollection *MtrlCollection)
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());
	if (Surface)
	{
		return Surface->GetResourceName();
	}
	return TEXT("");
}

void FModelMaterialRef::Serialize(FArchive &Ar, uint32 Ver)
{
	FModelMaterial::Serialize(Ar, Ver);

	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << BaseSkuid;
	}
}

UMaterialInterface *FModelMaterialRef::GetUE4Material()
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());

	if (Surface!=NULL)
	{
		return Surface->GetUE4Material();
	}
	
	return NULL;
}

void FModelMaterialRef::UpdateParameters(UMaterialInstanceDynamic *Mtrl, TArray<FModelTexture *> &Textures)
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());
	if (Surface != NULL)
	{
		Surface->UpdateParameters(Mtrl);
	}
	FModelMaterial::UpdateParameters(Mtrl, Textures);
}

void FModelMaterialRef::CopyOverrideParametersTo(FModelMaterial *Material)
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());
	if (Surface != NULL)
	{
		Surface->CopyOverrideParameters(Material);
	}
	FModelMaterial::CopyOverrideParametersTo(Material);
}

void FModelMaterialRef::GetRefTextures(TArray<FTexSlotInfo> &RefTextures, bool bExcludeDeprecated)
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());
	
	if (Surface != NULL && Surface->m_MaterialInfo)
	{
		Surface->m_MaterialInfo->GetRefTextures(RefTextures, bExcludeDeprecated);
	}
}

void FModelMaterialRef::GetRefValues(TArray<FScalarSlotInfo> &RefValues, bool bExcludeDeprecated)
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());

	if (Surface != NULL && Surface->m_MaterialInfo)
	{
		Surface->m_MaterialInfo->GetRefValues(RefValues, bExcludeDeprecated);
		for (int32 i = 0; i < RefValues.Num(); ++i)
		{
			int32 FoundIndex = FindFloatIndex(RefValues[i].ParamName);
			if (FoundIndex != INDEX_NONE)
			{
				float Value = GetFloatValue(RefValues[i].ParamName);
				RefValues[i].FloatValue = Value;
			}
		}
	}
}

void FModelMaterialRef::GetRefValues(TArray<FVectorSlotInfo> &RefValues, bool bExcludeDeprecated)
{
	USurfaceFile *Surface = Cast<USurfaceFile>(RefSurface.Get());

	if (Surface != NULL && Surface->m_MaterialInfo)
	{
		Surface->m_MaterialInfo->GetRefValues(RefValues, bExcludeDeprecated);
		for (int32 i = 0; i < RefValues.Num(); ++i)
		{
			int32 FoundIndex = FindVectorIndex(RefValues[i].ParamName);
			if (FoundIndex != INDEX_NONE)
			{
				FLinearColor Value = GetVectorValue(RefValues[i].ParamName);
				RefValues[i].LinearValue = Value;
			}
		}
	}
}



