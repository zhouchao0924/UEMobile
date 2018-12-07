
#include "SurfaceFile.h"
#include "ModelCompress.h"
#include "ResourceMgr.h"
#include "Materials/MaterialInstanceDynamic.h"

void USurfaceFile::FHeader::Serialize(FArchive &Ar)
{
	FResourceSummary::Serialize(Ar);

	if (Ar.IsSaving())
	{
		int32 NumTex = Textures.Num();
		Ar << NumTex;
		for (int32 i = 0; i < NumTex; ++i)
		{
			Textures[i].Serialize(Ar, BodyVersion);
		}

		Ar << Image.SizeX;
		Ar << Image.SizeY;

		int32 NumBytes = Image.Data.Num();
		Ar << NumBytes;
		if (NumBytes > 0)
		{
			Ar.Serialize(Image.Data.GetData(), NumBytes);
		}
	}
	else if (Ar.IsLoading())
	{
		int32 NumTex = 0;
		Ar << NumTex;
		Textures.SetNum(NumTex);
		for (int32 i = 0; i < NumTex; ++i)
		{
			Textures[i].Serialize(Ar, BodyVersion);
		}
		
		if (HeadVersion > 3)
		{
			if (HeadVersion > 4)
			{
				Ar << Image.SizeX;
				Ar << Image.SizeY;
			}

			int32 NumBytes = 0;
			Ar << NumBytes;
			Image.Data.SetNum(NumBytes);
			if (NumBytes > 0)
			{
				Ar.Serialize(Image.Data.GetData(), NumBytes);
			}
		}
	}

}

USurfaceFile::USurfaceFile(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	,m_MaterialInfo(NULL)
{
}

int32 USurfaceFile::GetNumerOfTexureSlot()
{
	TArray<FTexSlotInfo> TexSlots;
	m_MaterialInfo->GetRefTextures(TexSlots, true);
	return TexSlots.Num();
}

int32 USurfaceFile::GetNumberOfScalarValues()
{
	TArray<FScalarSlotInfo> ValueSlots;
	m_MaterialInfo->GetRefValues(ValueSlots, true);
	return ValueSlots.Num();
}

float USurfaceFile::GetScalarValue(int32 iSlot, FString & Name)
{
	TArray<FScalarSlotInfo> ValueSlots;
	m_MaterialInfo->GetRefValues(ValueSlots, true);
	if (ValueSlots.IsValidIndex(iSlot))
	{
		FScalarSlotInfo &Slot = ValueSlots[iSlot];
		Name = Slot.ParamName.ToString();

		int32 FoundIndex = m_MaterialInfo->FindFloatIndex(Slot.ParamName);
		if (FoundIndex != INDEX_NONE)
		{
			return m_MaterialInfo->GetFloatValue(Slot.ParamName);
		}
		return Slot.FloatValue;
	}
	return 0.0f;
}

void USurfaceFile::SetScalarValue(int32 iSlot, float Value)
{
	TArray<FScalarSlotInfo> ValueSlots;
	m_MaterialInfo->GetRefValues(ValueSlots, true);
	if (ValueSlots.IsValidIndex(iSlot))
	{
		FScalarSlotInfo &Slot = ValueSlots[iSlot];
		if (Slot.FloatValue != Value)
		{
			m_MaterialInfo->SetFloatValue(Slot.ParamName, Value);
			MarkDirty();
		}
	}
}

int32 USurfaceFile::GetNumberOfVectorValues()
{
	TArray<FVectorSlotInfo> ValueSlots;
	m_MaterialInfo->GetRefValues(ValueSlots, true);
	return ValueSlots.Num();
}

FLinearColor USurfaceFile::GetVectorValue(int32 iSlot, FString & Name)
{
	TArray<FVectorSlotInfo> ValueSlots;
	m_MaterialInfo->GetRefValues(ValueSlots, true);
	if (ValueSlots.IsValidIndex(iSlot))
	{
		FVectorSlotInfo &Slot = ValueSlots[iSlot];
		Name = Slot.ParamName.ToString();

		int32 FoundIndex = m_MaterialInfo->FindVectorIndex(Slot.ParamName);
		if (FoundIndex != INDEX_NONE)
		{
			return m_MaterialInfo->GetVectorValue(Slot.ParamName);
		}
		return Slot.LinearValue;
	}
	return FLinearColor::Black;
}

void USurfaceFile::SetVectorValue(int32 iSlot, const FLinearColor & Value)
{
	TArray<FVectorSlotInfo> ValueSlots;
	m_MaterialInfo->GetRefValues(ValueSlots, true);
	if (ValueSlots.IsValidIndex(iSlot))
	{
		FVectorSlotInfo &Slot = ValueSlots[iSlot];
		if (Slot.LinearValue != Value)
		{
			m_MaterialInfo->SetVectorValue(Slot.ParamName, Value);
			MarkDirty();
		}
	}
}

UMaterialInstanceDynamic * USurfaceFile::GetDynamicMaterialInstance()
{
	UMaterialInstanceDynamic *DynamicMtrl = NULL;

	FModelMaterial *material = GetMaterial();
	if (material)
	{
		UMaterialInterface *ue4Mtrl = material->GetUE4Material();
		if (ue4Mtrl)
		{
			DynamicMtrl = UMaterialInstanceDynamic::Create(ue4Mtrl, this);
			material->UpdateParameters(DynamicMtrl, m_Textures);
		}
	}

	return DynamicMtrl;
}

void USurfaceFile::BeginDestroy()
{
	Clean();
	Super::BeginDestroy();
}

void USurfaceFile::SerializeHeader(FArchive &Ar)
{
	Super::SerializeHeader(Ar);

	if (Ar.IsSaving())
	{
		uint32 type = m_MaterialInfo ? m_MaterialInfo->GetType() : EMaterialNone;
		Ar << type;
		if (m_MaterialInfo)
		{
			m_MaterialInfo->Serialize(Ar, m_Header.BodyVersion);
		}
	}
	else if(Ar.IsLoading())
	{
		uint32 type = 0;
		Ar << type;

		m_MaterialInfo = new FModelMaterialUE4();
		if (m_MaterialInfo)
		{
			m_MaterialInfo->Serialize(Ar, m_Header.BodyVersion);
		}
	}
	
}

void  USurfaceFile::Serialize(FArchive &Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaving())
	{
		int64 HeaderPos = Ar.Tell();

		int32 NumTextures = m_Textures.Num();
		m_Header.Textures.SetNum(NumTextures);

		SerializeHeader(Ar);

		//serialze texture
		for (int32 iTexture = 0; iTexture < m_Header.Textures.Num(); ++iTexture)
		{
			FChunk &texChunk = m_Header.Textures[iTexture];
			texChunk.Offset = (int32)Ar.Tell();
			m_Textures[iTexture]->Serialize(Ar, m_Header.BodyVersion);
			texChunk.Size = int32(Ar.Tell() - texChunk.Offset);
		}

		int64 TailPos = Ar.Tell();
		Ar.Seek(HeaderPos);

		SerializeHeader(Ar);

		Ar.Seek(TailPos);
	}
	else if (Ar.IsLoading())
	{
		SerializeHeader(Ar);

		//serialze texture
		for (int32 iTexture = 0; iTexture < m_Header.Textures.Num(); ++iTexture)
		{
			FChunk texChunk = m_Header.Textures[iTexture];
			Ar.Seek(texChunk.Offset);
			FModelTexture *newTexutre = new FModelTexture();
			newTexutre->Serialize(Ar, m_Header.BodyVersion);
			m_Textures.Add(newTexutre);
		}
	}
	
}

void USurfaceFile::Unload()
{
	Clean();
	Super::Unload();
}

void USurfaceFile::SerizlizeHistory(FArchive &Ar, uint32 HistoryFileVer)
{
	//serialize surface
}

void USurfaceFile::Clean()
{
	for (int32 i = 0; i < m_Textures.Num(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}

	m_Textures.Empty();
}

FModelMaterial * USurfaceFile::GetMaterial()
{
	return m_MaterialInfo;
}

UMaterialInterface *USurfaceFile::GetUE4Material()
{
	if (m_MaterialInfo)
	{
		return m_MaterialInfo->GetUE4Material();
	}
	return NULL;
}

void USurfaceFile::UpdateParameters(UMaterialInstanceDynamic *Mtrl)
{
	if (Mtrl && m_MaterialInfo)
	{
		m_MaterialInfo->UpdateParameters(Mtrl, m_Textures);
	}
}

void USurfaceFile::CopyOverrideParameters(FModelMaterial *Material)
{
	if (m_MaterialInfo)
	{
		m_MaterialInfo->CopyOverrideParametersTo(Material);
	}
}

FModelMaterialUE4 *USurfaceFile::GetMaterialUE4()
{
	if (m_MaterialInfo && m_MaterialInfo->GetType() == EMaterialUE4)
	{
		return (FModelMaterialUE4 *)m_MaterialInfo;
	}
	return NULL;
}

void USurfaceFile::SetType(EMaterialType InType)
{
	if (m_MaterialInfo != NULL && m_MaterialInfo->GetType()!=InType)
	{
		SAFE_DELETE(m_MaterialInfo);
	}

	if (!m_MaterialInfo)
	{
		if (InType == EMaterialUE4)
		{
			m_MaterialInfo = new FModelMaterialUE4();
		}
	}
}

void USurfaceFile::SetSurface(FModelMaterial *Material, TArray<FModelTexture *> &Textures)
{
	if (Material->GetType() == EMaterialRef)
	{
		Clean();

		FModelMaterialUE4 *newMtrlUE4 = NULL;
		FModelMaterialRef *mtrlUE4 = (FModelMaterialRef *)Material;
		
		USurfaceFile *Surface = Cast<USurfaceFile>(mtrlUE4->RefSurface.Get());
		if (Surface && Surface->m_MaterialInfo)
		{
			newMtrlUE4 = new FModelMaterialUE4();
			newMtrlUE4->Name = Surface->m_MaterialInfo->Name;
			newMtrlUE4->UE4Path = Surface->m_MaterialInfo->UE4Path;

			for (int32 i = 0; i < Surface->m_MaterialInfo->OverrideTextureParameters.Num(); ++i)
			{
				FSurfaceParameterTexture &SurfTexParam = Surface->m_MaterialInfo->OverrideTextureParameters[i];
				int32 FoundIndex = newMtrlUE4->FindTextureIndex(SurfTexParam.ParamName);
				if (FoundIndex == INDEX_NONE)
				{
					int32 Value = m_Textures.Num();
					newMtrlUE4->SetTextureValue(SurfTexParam.ParamName, Value);

					FModelTexture *surfTex = Surface->m_Textures[SurfTexParam.Value];
					FModelTexture *newTex = new FModelTexture();
					newTex->CopyFrom(*surfTex);
					m_Textures.Add(newTex);
				}
			}
		}

		for (int32 i = 0; i < Material->OverrideTextureParameters.Num(); ++i)
		{
			FSurfaceParameterTexture &TexParam = Material->OverrideTextureParameters[i];

			if (Textures.IsValidIndex(TexParam.Value))
			{
				FModelTexture *tex = Textures[TexParam.Value];
				check(tex != nullptr);

				FModelTexture *newTex = new FModelTexture();
				newTex->CopyFrom(*tex);

				int32 Value = m_Textures.Num();
				newMtrlUE4->SetTextureValue(TexParam.ParamName, Value);

				m_Textures.Add(newTex);
			}
		}

		Material->CopyOverrideParametersTo(newMtrlUE4);

		SAFE_DELETE(Material);
		m_MaterialInfo = newMtrlUE4;
	}
	else
	{
		Clean();

		for (int32 i = 0; i < Material->OverrideTextureParameters.Num(); ++i)
		{
			FSurfaceParameterTexture &TexParam = Material->OverrideTextureParameters[i];
			
			if (TexParam.Value != INDEX_NONE)
			{
				FModelTexture *tex = Textures[TexParam.Value];
				check(tex != nullptr);

				FModelTexture *newTex = new FModelTexture();
				newTex->CopyFrom(*tex);

				TexParam.Value = m_Textures.Num();;
				m_Textures.Add(newTex);
			}
		}

		m_MaterialInfo = (FModelMaterialUE4 *)Material;
	}
}

void USurfaceFile::CopyOverrideTextures(FModelMaterial *Material, TArray<FModelTexture *> &Textures)
{
	for (int32 i = 0; i < Material->OverrideTextureParameters.Num(); ++i)
	{
		FSurfaceParameterTexture &TexParam = Material->OverrideTextureParameters[i];

		if (TexParam.Value != INDEX_NONE)
		{
			FModelTexture *tex = Textures[TexParam.Value];
			check(tex != nullptr);

			FModelTexture *newTex = new FModelTexture();
			newTex->CopyFrom(*tex);

			TexParam.Value = m_Textures.Num();;
			m_Textures.Add(newTex);
		}
	}
}

FModelTexture *USurfaceFile::GetTextureByName(const FName &Name)
{
	int32 TexValue = m_MaterialInfo->GetTextureValue(Name);

	if (m_Textures.IsValidIndex(TexValue))
	{
		return m_Textures[TexValue];
	}

	return NULL;
}

void USurfaceFile::CheckResource()
{
	for (int32 i = 0; i < m_Textures.Num(); ++i)
	{
		FModelTexture *tex = m_Textures[i];
		if (tex && tex->Source.CompressedImages.Num() == 0)
		{
			CompressUtil::CompressTexture(tex);
		}
	}
	Super::CheckResource();
}

