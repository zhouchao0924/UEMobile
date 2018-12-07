
#include "ModelFile.h"
#include "ModelCompress.h"
#include "ResourceMgr.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Materials/MaterialInstanceDynamic.h"

FMaterialSlot::FMaterialSlot()
	: iModel(INDEX_NONE)
	, MaterialIndex(INDEX_NONE)
	, iSection(INDEX_NONE)
{
}

//////////////////////////////////////////////////////////////////////////
UModelFile::UModelFile(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	, EyeLoc(FVector::ZeroVector)
	, FocusLoc(FVector::ZeroVector)
	, Scale3D(1.0f)
	, EyeDistance(0)
	, DepthInMM(0)
	, WidthInMM(0)
	, HeightInMM(0)
	, CenterType(ECenterAdjustType::Original)
{
}

void UModelFile::FHeader::Serialize(FArchive &Ar)
{
	FResourceSummary::Serialize(Ar);

	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Image.SizeX;
		Ar << Image.SizeY;

		if (Ar.IsSaving())
		{
			int32 NumBytes = Image.Data.Num();
			Ar << NumBytes;
			if (NumBytes > 0)
			{
				Ar.Serialize(Image.Data.GetData(), NumBytes);
			}
		}
		else if (Ar.IsLoading())
		{
			int32 NumBytes = 0;
			Ar << NumBytes;
			Image.Data.SetNum(NumBytes);
			if (NumBytes > 0)
			{
				Ar.Serialize(Image.Data.GetData(), NumBytes);
			}
		}
	}

	if (Ar.IsSaving())
	{
		int32 NumModel = Models.Num();
		Ar << NumModel;
		for (int32 i = 0; i < NumModel; ++i)
		{
			Models[i].Serialize(Ar, BodyVersion);
		}

		int32 NumMtrl = Materials.Num();
		Ar << NumMtrl;
		for (int32 i = 0; i < NumMtrl; ++i)
		{
			Materials[i].Serialize(Ar, BodyVersion);
		}

		int32 NumTex = Textures.Num();
		Ar << NumTex;
		for (int32 i = 0; i < NumTex; ++i)
		{
			Textures[i].Serialize(Ar, BodyVersion);
		}
	}
	else if (Ar.IsLoading())
	{
		int32 NumModel = 0;
		Ar << NumModel;
		Models.SetNum(NumModel);
		for (int32 i = 0; i < NumModel; ++i)
		{
			Models[i].Serialize(Ar, BodyVersion);
		}

		int32 NumMtrl = 0;
		Ar << NumMtrl;
		Materials.SetNum(NumMtrl);
		for (int32 i = 0; i < NumMtrl; ++i)
		{
			Materials[i].Serialize(Ar, BodyVersion);
		}

		int32 NumTex = 0;
		Ar << NumTex;
		Textures.SetNum(NumTex);
		for (int32 i = 0; i < NumTex; ++i)
		{
			Textures[i].Serialize(Ar, BodyVersion);
		}
	}
}

void UModelFile::Serialize(FArchive &Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaving())
	{
		int64 HeaderPos = Ar.Tell();

		int32 NumModels = m_Models.Num();
		m_Header.Models.SetNum(NumModels);

		int32 NumMaterials = m_Materials.Num();
		m_Header.Materials.SetNum(NumMaterials);

		int32 NumTextures = m_Textures.Num();
		m_Header.Textures.SetNum(NumTextures);

		SerializeHeader(Ar);

		//serialize models
		for (int32 iModel = 0; iModel < NumModels; ++iModel)
		{
			FChunk &modelChunk = m_Header.Models[iModel];
			modelChunk.Offset = (int32)Ar.Tell();
			m_Models[iModel]->Serialize(Ar, m_Header.BodyVersion);
			modelChunk.Size = int32(Ar.Tell() - modelChunk.Offset);
		}

		//serialize materials
		for (int32 iMaterial = 0; iMaterial < NumMaterials; ++iMaterial)
		{
			FChunk &mtrlChunk = m_Header.Materials[iMaterial];
			mtrlChunk.Offset = (int32)Ar.Tell();

			uint32 type = (uint32)m_Materials[iMaterial]->GetType();
			Ar << type;

			m_Materials[iMaterial]->Serialize(Ar, m_Header.BodyVersion);
			mtrlChunk.Size = int32(Ar.Tell() - mtrlChunk.Offset);
		}

		//serialze texture
		for (int32 iTexture = 0; iTexture < m_Header.Textures.Num(); ++iTexture)
		{
			FChunk &texChunk = m_Header.Textures[iTexture];
			texChunk.Offset = (int32)Ar.Tell();
			m_Textures[iTexture]->Serialize(Ar, m_Header.BodyVersion);
			texChunk.Size = int32(Ar.Tell() - texChunk.Offset);
		}

		Ar << LocalBounds;
		Ar << EyeLoc;
		Ar << FocusLoc;
		Ar << EyeDistance;

		Ar << CenterType;
		Ar << Offset;
		Ar << DepthInMM;
		Ar << WidthInMM;
		Ar << HeightInMM;

		int64 TailPos = Ar.Tell();
		Ar.Seek(HeaderPos);

		SerializeHeader(Ar);

		Ar.Seek(TailPos);
	}
	else if (Ar.IsLoading())
	{
		SerializeHeader(Ar);

		//serialize models
		for (int32 iModel = 0; iModel < m_Header.Models.Num(); ++iModel)
		{
			FChunk modelChunk = m_Header.Models[iModel];
			Ar.Seek(modelChunk.Offset);
			FModel *newModel = new FModel();
			newModel->Serialize(Ar, m_Header.BodyVersion);
			m_Models.Add(newModel);
		}

		//serialize materials
		for (int32 iMaterial = 0; iMaterial < m_Header.Materials.Num(); ++iMaterial)
		{
			FChunk mtrlChunk = m_Header.Materials[iMaterial];
			Ar.Seek(mtrlChunk.Offset);

			uint32 type = 0;
			Ar << type;

			FModelMaterial *newMtrl = FModelMaterial::CreateMaterial((EMaterialType)type);
			if (newMtrl != NULL)
			{
				newMtrl->Serialize(Ar, m_Header.BodyVersion);
			}

			m_Materials.Add(newMtrl);
		}

		//serialze texture
		for (int32 iTexture = 0; iTexture < m_Header.Textures.Num(); ++iTexture)
		{
			FChunk texChunk = m_Header.Textures[iTexture];
			Ar.Seek(texChunk.Offset);
			FModelTexture *newTexutre = new FModelTexture();
			newTexutre->Serialize(Ar, m_Header.BodyVersion);
			m_Textures.Add(newTexutre);
		}

		Ar << LocalBounds;
		Ar << EyeLoc;
		Ar << FocusLoc;
		Ar << EyeDistance;

		if (m_Header.BodyVersion > MODELFILE_BODY_VER_3)
		{
			Ar << CenterType;
			Ar << Offset;
		}

		if (m_Header.BodyVersion == MODELFILE_BODY_VER_5)
		{
			Ar << Scale3D;
			ResetSize();
		}
		else if (m_Header.BodyVersion > MODELFILE_BODY_VER_5)
		{
			Ar << DepthInMM;
			Ar << WidthInMM;
			Ar << HeightInMM;
			CaclScale3D();
		}
		else
		{
			ResetSize();
		}
	}
}

void UModelFile::Unload()
{
	Clean();
	Super::Unload();
}

void UModelFile::SerizlizeHistory(FArchive &Ar, uint32 HistoryFileVer)
{
	//load history....
}

FModel *UModelFile::NewModel()
{
	FModel *Model = new FModel();
	m_Models.Add(Model);
	return Model;
}

FModelTexture *UModelFile::NewTexture()
{
	FModelTexture *Tex = new FModelTexture();
	m_Textures.Add(Tex);
	return Tex;
}

FModelMaterialUE4 *UModelFile::NewMaterial()
{
	FModelMaterialUE4 *Mtrl = new FModelMaterialUE4();
	m_Materials.Add(Mtrl);
	return Mtrl;
}

void UModelFile::Clean()
{
	for (int32 i = 0; i < m_Models.Num(); ++i)
	{
		SAFE_DELETE(m_Models[i]);
	}

	for (int32 i = 0; i < m_Materials.Num(); ++i)
	{
		SAFE_DELETE(m_Materials[i]);
	}

	for (int32 i = 0; i < m_Textures.Num(); ++i)
	{
		SAFE_DELETE(m_Textures[i]);
	}

	m_Models.Empty();
	m_Textures.Empty();
	m_Materials.Empty();
}

void UModelFile::BeginDestroy()
{
	Clean();
	Super::BeginDestroy();
}

void  UModelFile::Compress()
{
	if (!m_Header.bCompressed)
	{
		for (int32 i = 0; i < m_Models.Num(); ++i)
		{
			FModel *model = m_Models[i];
			check(model != NULL);
			CompressUtil::CompressModel(model);
		}
		m_Header.bCompressed = 1;
	}
}

void UModelFile::GetMaterialRefTextures(FModelMaterial *Material, const TArray<FModelTexture*> &Texs, TArray<FModelTexture*> &RefTexs)
{
	TArray<FTexSlotInfo> RefTexIndices;
	Material->GetRefTextures(RefTexIndices, true);

	for (int32 i = 0; i < RefTexIndices.Num(); ++i)
	{
		FTexSlotInfo &Slot = RefTexIndices[i];

		if (Texs.IsValidIndex(Slot.iTex))
		{
			RefTexs[Slot.iSlot] = Texs[Slot.iTex];
		}
	}
}

USurfaceFile *UModelFile::ExtractSurface(int32 iSurface, const FSurfaceInfo &ChineseName, const FSurfaceInfo &EnglishName)
{
	USurfaceFile *Surface = NULL;

	if (m_Materials.IsValidIndex(iSurface))
	{
		FModelMaterial *Material = m_Materials[iSurface];

		Surface = ResMgr->NewSurface();
		Surface->SetResourceName(EnglishName.Name);

		if (Surface != NULL)
		{
			FModelMaterialRef *RefMaterial = new FModelMaterialRef();
			RefMaterial->BaseSkuid = Surface->GetResID();
			RefMaterial->RefSurface = Surface;

			Surface->SetSurface(Material, m_Textures);

			m_Materials[iSurface] = RefMaterial;
			Surface->DRInfo = UVaRestJsonObject::ConstructJsonObject(this);
			Surface->DRInfo->SetStringField("ChineseName", ChineseName.Name);
			ClearResource();
			UpdateDepends();
		}
	}

	return Surface;
}

void UModelFile::UpdateDepends()
{
	m_Header.Dependences.Empty();
	for (int32 i = 0; i < m_Materials.Num(); ++i)
	{
		FModelMaterial *modelMaterial = m_Materials[i];
		if (modelMaterial->GetType() == EMaterialRef)
		{
			FModelMaterialRef *refMtrl = (FModelMaterialRef*)modelMaterial;
			m_Header.Dependences.AddUnique(refMtrl->BaseSkuid);
		}
	}
}

const TArray<FString> * UModelFile::GetDepends()
{
	return &m_Header.Dependences;
}

void UModelFile::OnPostLoad()
{
	Super::OnPostLoad();

	if (ResMgr)
	{
		for (int32 i = 0; i < m_Materials.Num(); ++i)
		{
			FModelMaterial *modelMaterial = m_Materials[i];
			if (modelMaterial->GetType() == EMaterialRef)
			{
				FModelMaterialRef *refMtrl = (FModelMaterialRef*)modelMaterial;
				if (!refMtrl->RefSurface.IsValid())
				{
					refMtrl->RefSurface = ResMgr->FindRes(refMtrl->BaseSkuid);
				}
			}
		}
	}
}

bool UModelFile::IsCookedPhysic()
{
	for (int32 i = 0; i < m_Models.Num(); ++i)
	{
		FModel *model = m_Models[i];
		if (model && model->AggGeom.ConvexElems.Num() <= 0)
		{
			return false;
		}
	}
	return true;
}

void UModelFile::CheckResource()
{
	for (int32 i = 0; i < m_Textures.Num(); ++i)
	{
		FModelTexture *tex = m_Textures[i];
		if (tex && tex->Source.CompressedImages.Num() == 0)
		{
			CompressUtil::CompressTexture(tex);
		}
	}
}

void UModelFile::ClearResource()
{
	TArray<int32> UsedTextures;
	for (int32 i = 0; i < m_Materials.Num(); ++i)
	{
		TArray<FTexSlotInfo> RefTextures;
		FModelMaterial *material = m_Materials[i];

		material->GetRefTextures(RefTextures, false);

		for (int32 iSlot = 0; iSlot < RefTextures.Num(); ++iSlot)
		{
			FTexSlotInfo &Slot = RefTextures[iSlot];
			if (Slot.iTex != INDEX_NONE)
			{
				UsedTextures.AddUnique(Slot.iTex);
			}
		}
	}

	TArray<int32> Remap;
	Remap.SetNum(m_Textures.Num());

	int32 k = 0;
	for (int32 i = 0; i < m_Textures.Num(); ++i)
	{
		int32 NewIndex = UsedTextures.Find(i);
		Remap[i] = NewIndex;
		if (NewIndex == INDEX_NONE)
		{
			SAFE_DELETE(m_Textures[i]);
		}
		else
		{
			m_Textures[k++] = m_Textures[i];
		}
	}
	m_Textures.SetNum(k);

	for (int32 i = 0; i < m_Materials.Num(); ++i)
	{
		FModelMaterial *material = m_Materials[i];
		material->RemapTexture(Remap);
	}
}

void UModelFile::GetSurfaces(TArray<USurfaceFile*>& Surfaces)
{
	for (int32 i = 0; i < m_Materials.Num(); ++i)
	{
		FModelMaterial *material = m_Materials[i];
		if (material && material->GetType() == EMaterialRef)
		{
			FModelMaterialRef *MaterialRef = (FModelMaterialRef *)material;
			USurfaceFile *Surface = Cast<USurfaceFile>(MaterialRef->RefSurface.Get());
			if (Surface)
			{
				Surfaces.Add(Surface);
			}
		}
	}
}

void UModelFile::GetMaterialInfo(int32 MaterialIndex, TArray<FMaterialSlot> &materialSlots)
{
	for (int32 m = 0; m < m_Models.Num(); ++m)
	{
		FModel *model = m_Models[m];
		for (int32 refIndex = 0; refIndex < model->RefMaterials.Num(); ++refIndex)
		{
			if (MaterialIndex == model->RefMaterials[refIndex])
			{
				FMaterialSlot *slot = new (materialSlots) (FMaterialSlot);
				slot->MaterialIndex = MaterialIndex;
				slot->iModel = m;
				slot->iSection = refIndex;
			}
		}
	}
}

int32 UModelFile::GetNumberOfMaterial()
{
	return m_Materials.Num();
}

int32 UModelFile::GetNumerOfTexureSlot(int32 iMaterial)
{
	TArray<FTexSlotInfo> texSlots;

	if (m_Materials.IsValidIndex(iMaterial))
	{
		FModelMaterial *mtrl = m_Materials[iMaterial];
		if (mtrl)
		{
			mtrl->GetRefTextures(texSlots, true);
		}
	}

	return texSlots.Num();
}

bool UModelFile::SetMaterialSlotByUE4Material(int32 MaterialIndex, UMaterialInterface *InMaterial)
{
	if (!m_Materials.IsValidIndex(MaterialIndex))
	{
		return false;
	}

	FModelMaterial *oldMaterial = m_Materials[MaterialIndex];
	if (oldMaterial)
	{
		delete oldMaterial;
	}

	FModelMaterialUE4 *newMaterial = new FModelMaterialUE4();
	if (newMaterial)
	{
		newMaterial->UE4Path = InMaterial->GetPathName();
	}

	m_Materials[MaterialIndex] = newMaterial;

	//ClearResource();
	//UpdateDepends();

	MarkDirty();

	return true;
}

bool UModelFile::SetMaterialSlotBySurface(int32 MaterialIndex, USurfaceFile *Surface)
{
	if (!m_Materials.IsValidIndex(MaterialIndex))
	{
		return false;
	}

	FModelMaterial *oldMaterial = m_Materials[MaterialIndex];
	if (oldMaterial)
	{
		delete oldMaterial;
	}

	FModelMaterialRef *newMaterial = new FModelMaterialRef();
	if (newMaterial)
	{
		newMaterial->RefSurface = Surface;
		newMaterial->BaseSkuid = Surface->GetResID();
	}

	m_Materials[MaterialIndex] = newMaterial;

	//ClearResource();
	//UpdateDepends();

	MarkDirty();

	return true;
}

void UModelFile::ResetMaterial(int32 MaterialIndex)
{
	if (!m_Materials.IsValidIndex(MaterialIndex))
	{
		return;
	}

	FModelMaterial *material = m_Materials[MaterialIndex];
	material->OverrideFloatParameters.Empty();
	material->OverrideVectorParameters.Empty();
	material->OverrideTextureParameters.Empty();

	ClearResource();
	UpdateDepends();
	MarkDirty();
}

FModelTexture *UModelFile::GetTextureBySlotType(int32 iMaterial, ETexSlot slotType)
{
	FName TargetName = *GetTexSlotName(slotType);
	return GetTextureByName(iMaterial, TargetName);
}

FModelTexture *UModelFile::GetTextureByName(FModelMaterial *modelMaterial, const FName &TargetName)
{
	FModelTexture *modelTex = NULL;
	int32 TexValue = modelMaterial->GetTextureValue(TargetName);

	if (m_Textures.IsValidIndex(TexValue))
	{
		modelTex = m_Textures[TexValue];
	}
	else
	{
		if (modelMaterial->GetType() == EMaterialUE4)
		{
			modelTex = NULL;
		}
		else if (modelMaterial->GetType() == EMaterialRef)
		{
			FModelMaterialRef *mtrlRef = (FModelMaterialRef*)modelMaterial;
			USurfaceFile *Surface = Cast<USurfaceFile>(mtrlRef->RefSurface.Get());
			if (Surface)
			{
				modelTex = Surface->GetTextureByName(TargetName);
			}
		}
	}

	return modelTex;
}

FModelTexture *UModelFile::GetTextureByName(int32 iMaterial, const FName &TargetName)
{
	FModelTexture *modelTex = NULL;

	if (m_Materials.IsValidIndex(iMaterial))
	{
		FModelMaterial *mtrl = m_Materials[iMaterial];
		if (mtrl)
		{
			modelTex = GetTextureByName(mtrl, TargetName);
		}
	}
	return modelTex;
}

FModelMaterial *UModelFile::GetMaterial(int32 MaterialIndex)
{
	FModelMaterial *material = NULL;

	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		material = m_Materials[MaterialIndex];
	}

	return material;
}

UMaterialInstanceDynamic *UModelFile::GetUE4Material(int32 MaterialIndex)
{
	UMaterialInstanceDynamic *DynamicMtrl = NULL;

	FModelMaterial *material = GetMaterial(MaterialIndex);
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


void UModelFile::SetPreviewCamera(const FVector &InEyeLoc, const FVector &InFocusLoc)
{
	EyeLoc = InEyeLoc;
	FocusLoc = InFocusLoc;
	EyeDistance = (EyeLoc - FocusLoc).Size();
	MarkDirty();
}

void UModelFile::GetPreviewCamera(FVector &OutEyeLoc, FVector &OutFocusLoc)
{
	if (EyeDistance <= 0)
	{
		FBoxSphereBounds Bounds(LocalBounds);
		FocusLoc = Bounds.Origin;
		EyeDistance = Bounds.SphereRadius;
		EyeLoc = FocusLoc + FVector(-1, 0, 1).GetSafeNormal()*EyeDistance;
	}

	OutEyeLoc = EyeLoc;
	OutFocusLoc = FocusLoc;
}



EMaterialType UModelFile::GetMaterialType(int32 MaterialIndex)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *mtrl = m_Materials[MaterialIndex];
		return mtrl->GetType();
	}
	return EMaterialType::EMaterialNone;
}

int32 UModelFile::GetNumberOfScalarValues(int32 MaterialIndex)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *Material = m_Materials[MaterialIndex];
		if (Material)
		{
			TArray<FScalarSlotInfo> ValueSlots;
			Material->GetRefValues(ValueSlots, true);
			return ValueSlots.Num();
		}
	}
	return 0;
}

float UModelFile::GetScalarValue(int32 MaterialIndex, int32 iSlot, FString &Name)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *Material = m_Materials[MaterialIndex];
		if (Material)
		{
			TArray<FScalarSlotInfo> ValueSlots;
			Material->GetRefValues(ValueSlots, true);
			if (ValueSlots.IsValidIndex(iSlot))
			{
				FScalarSlotInfo &Slot = ValueSlots[iSlot];
				Name = Slot.ParamName.ToString();

				int32 FoundIndex = Material->FindFloatIndex(Slot.ParamName);
				if (FoundIndex != INDEX_NONE)
				{
					return Material->GetFloatValue(Slot.ParamName);
				}
				return Slot.FloatValue;
			}
		}
	}
	return 0;
}

void  UModelFile::SetScalarValue(int32 MaterialIndex, int32 iSlot, float Value)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *Material = m_Materials[MaterialIndex];
		if (Material)
		{
			TArray<FScalarSlotInfo> ValueSlots;
			Material->GetRefValues(ValueSlots, true);
			if (ValueSlots.IsValidIndex(iSlot))
			{
				FScalarSlotInfo &Slot = ValueSlots[iSlot];
				if (Slot.FloatValue != Value)
				{
					Material->SetFloatValue(Slot.ParamName, Value);
				}
			}
			MarkDirty();
		}
	}
}

int32 UModelFile::GetNumberOfVectorValues(int32 MaterialIndex)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *Material = m_Materials[MaterialIndex];
		if (Material)
		{
			TArray<FVectorSlotInfo> ValueSlots;
			Material->GetRefValues(ValueSlots, true);
			return ValueSlots.Num();
		}
	}
	return 0;
}

FLinearColor UModelFile::GetVectorValue(int32 MaterialIndex, int32 iSlot, FString &Name)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *Material = m_Materials[MaterialIndex];
		if (Material)
		{
			TArray<FVectorSlotInfo> ValueSlots;
			Material->GetRefValues(ValueSlots, true);
			if (ValueSlots.IsValidIndex(iSlot))
			{
				FVectorSlotInfo &Slot = ValueSlots[iSlot];
				Name = Slot.ParamName.ToString();

				int32 FoundIndex = Material->FindVectorIndex(Slot.ParamName);
				if (FoundIndex != INDEX_NONE)
				{
					return Material->GetVectorValue(Slot.ParamName);
				}
				return Slot.LinearValue;
			}
		}
	}
	return FLinearColor::Black;
}

void  UModelFile::SetVectorValue(int32 MaterialIndex, int32 iSlot, const FLinearColor &Value)
{
	if (m_Materials.IsValidIndex(MaterialIndex))
	{
		FModelMaterial *Material = m_Materials[MaterialIndex];
		if (Material)
		{
			TArray<FVectorSlotInfo> ValueSlots;
			Material->GetRefValues(ValueSlots, true);
			if (ValueSlots.IsValidIndex(iSlot))
			{
				FVectorSlotInfo &Slot = ValueSlots[iSlot];
				if (Slot.LinearValue != Value)
				{
					Material->SetVectorValue(Slot.ParamName, Value);
				}
			}
			MarkDirty();
		}
	}
}

void UModelFile::CloneModel(UModelFile *OtherModel)
{
	if (OtherModel)
	{
		int32 otherBodyVer = OtherModel->GetSummary()->BodyVersion;
		FResourceSummary *Summary = GetSummary();
		int32 bodyVer = Summary->BodyVersion;

		for (int32 i = 0; i < m_Models.Num(); ++i)
		{
			FModel *model = m_Models[i];
			if (model)
			{
				delete model;
			}
		}
		m_Models.Empty();

		int32 numModel = OtherModel->m_Models.Num();
		if (numModel > 0)
		{
			m_Models.SetNum(numModel);
			for (int32 i = 0; i < numModel; ++i)
			{
				FModel *newModel = new FModel();
				m_Models[i] = newModel;

				FModel *model = OtherModel->m_Models[i];
				if (model)
				{
					TArray<uint8> Data;
					FMemoryWriter ArWriter(Data);
					model->Serialize(ArWriter, bodyVer);

					FMemoryReader ArReader(Data);
					newModel->Serialize(ArReader, bodyVer);
				}
			}
		}

		LocalBounds = OtherModel->LocalBounds;
		int32 curNumMaterial = m_Materials.Num();
		int32 numMtrl = OtherModel->m_Materials.Num();

		if (curNumMaterial < numMtrl)
		{
			for (int32 i = 0; i < (numMtrl - curNumMaterial); ++i)
			{
				NewMaterial();
			}
			curNumMaterial = m_Materials.Num();
		}

		for (int32 i = 0; i < numModel; ++i)
		{
			FModel *model = m_Models[i];
			for (int imtrl = 0; imtrl < model->RefMaterials.Num(); ++imtrl)
			{
				int32 mtrlIndex = model->RefMaterials[imtrl];
				if (!m_Materials.IsValidIndex(mtrlIndex))
				{
					model->RefMaterials[imtrl] = m_Materials.Num() - 1;
				}
			}
		}

		if (curNumMaterial > numMtrl)
		{
			int32 numDel = m_Materials.Num() - numMtrl;
			for (int i = 0; i < numDel; ++i)
			{
				FModelMaterial *delMtrl = m_Materials.Pop();
				if (delMtrl)
				{
					delete delMtrl;
				}
			}

			ClearResource();
			UpdateDepends();
		}
	}
}

FVector VecChangedHand(const FVector &Vec)
{
	return FVector(-Vec.X, Vec.Y, Vec.Z);
}

void UModelFile::ChangeHand()
{
	LocalBounds = FBox();

	for (int32 i = 0; i < m_Models.Num(); ++i)
	{
		FModel *model = m_Models[i];
		for (int32 iSection = 0; iSection < model->Sections.Num(); ++iSection)
		{
			FProcMeshSection &section = model->Sections[iSection];
			for (int32 iVert = 0; iVert < section.ProcVertexBuffer.Num(); ++iVert)
			{
				FProcMeshVertex &Vert = section.ProcVertexBuffer[iVert];
				Vert.Normal = VecChangedHand(Vert.Normal);
				Vert.Position = VecChangedHand(Vert.Position);
				Vert.Tangent.TangentX = VecChangedHand(Vert.Tangent.TangentX);
				LocalBounds += Vert.Position;
			}

			int32 NumTriangle = section.ProcIndexBuffer.Num() / 3;
			for (int32 TriIndex = 0; TriIndex < NumTriangle; ++TriIndex)
			{
				Swap<uint32>(section.ProcIndexBuffer[3 * TriIndex], section.ProcIndexBuffer[3 * TriIndex + 1]);
			}
		}
	}
	ResetSize();
	MarkDirty();
}

void UModelFile::SetCenterType(ECenterAdjustType InCenterType)
{
	FVector NewOffset = GetOffsetByCenterType(InCenterType);
	CenterType = InCenterType;
	SetOffset(NewOffset);
}

void UModelFile::ResetSize()
{
	FVector Size = LocalBounds.GetSize()*Scale3D*10.0f;
	DepthInMM = FMath::RoundToInt(Size.X);
	WidthInMM = FMath::RoundToInt(Size.Y);
	HeightInMM = FMath::RoundToInt(Size.Z);
}

void UModelFile::CaclScale3D()
{
	FBox Bounds = GetLocalBounds();
	FVector Size = Bounds.GetSize();
	Scale3D.X = DepthInMM *0.1f/ Size.X;
	Scale3D.Y = WidthInMM *0.1f/ Size.Y;
	Scale3D.Z = HeightInMM*0.1f/ Size.Z;
	MarkDirty();
}

void UModelFile::SetSizeInMM(int32 InDepth, int32 InWidth, int32 InHeight)
{
	DepthInMM = InDepth;
	WidthInMM = InWidth;
	HeightInMM = InHeight;
	CaclScale3D();
}

void UModelFile::SetOffset(const FVector &InOffset)
{
	Offset = InOffset;
	MarkDirty();
}

FBox UModelFile::GetLocalBounds()
{
	FTransform Rotation(FRotator(0, 180.0f, 0));
	FBox NewBox = LocalBounds.TransformBy(Rotation);
	return NewBox;
}

FVector UModelFile::GetOffsetByCenterType(ECenterAdjustType InCenterType)
{
	FVector NewOffset;

	if (InCenterType == ECenterAdjustType::Original)
	{
		NewOffset = FVector::ZeroVector;
	}
	else
	{
		FBox	Bounds = GetLocalBounds();
		FVector OldCenter = Bounds.GetCenter();
		FVector NewCenter;
		FVector Size = Bounds.GetExtent();

		switch (InCenterType)
		{
		case ECenterAdjustType::BottomCenter:
		{
			NewCenter = FVector(0, 0, Size.Z);
			break;
		}
		case ECenterAdjustType::TopCenter:
		{
			NewCenter = FVector(0, 0, -Size.Z);
			break;
		}
		case ECenterAdjustType::LeftCenter:
		{
			NewCenter = FVector(0, Size.Y, 0);
			break;
		}
		case ECenterAdjustType::RightCenter:
		{
			NewCenter = FVector(0, -Size.Y, 0);
			break;
		}
		case ECenterAdjustType::FrontCenter:
		{
			NewCenter = FVector(Size.X, 0, 0);
			break;
		}
		case ECenterAdjustType::BackCenter:
		{
			NewCenter = FVector(-Size.X, 0, 0);
			break;
		}
		case ECenterAdjustType::TopFrontEdgeCenter:
		{
			NewCenter = FVector(Size.X, 0, -Size.Z);
			break;
		}
		case ECenterAdjustType::TopBackEdgeCenter:
		{
			NewCenter = FVector(-Size.X, 0, -Size.Z);
			break;
		}
		case ECenterAdjustType::TopLeftEdgeCenter:
		{
			NewCenter = FVector(0, Size.Y, -Size.Z);
			break;
		}
		case ECenterAdjustType::TopRightEdgeCenter:
		{
			NewCenter = FVector(0, -Size.Y, -Size.Z);
			break;
		}
		case ECenterAdjustType::BottomFrontEdgeCenter:
		{
			NewCenter = FVector(Size.X, 0, Size.Z);
			break;
		}
		case ECenterAdjustType::BottomBackEdgeCenter:
		{
			NewCenter = FVector(-Size.X, 0, Size.Z);
			break;
		}
		case ECenterAdjustType::BottomLeftEdgeCenter:
		{
			NewCenter = FVector(0, Size.Y, Size.Z);
			break;
		}
		case ECenterAdjustType::BottomRightEdgeCenter:
		{
			NewCenter = FVector(0, -Size.Y, Size.Z);
			break;
		}
		case ECenterAdjustType::LeftFrontEdgeCenter:
		{
			NewCenter = FVector(Size.X, Size.Y, 0);
			break;
		}
		case ECenterAdjustType::RightFrontEdgeCenter:
		{
			NewCenter = FVector(Size.X, -Size.Y, 0);
			break;
		}
		case ECenterAdjustType::LeftBackEdgeCenter:
		{
			NewCenter = FVector(-Size.X, Size.Y, 0);
			break;
		}
		case ECenterAdjustType::RightBackEdgeCenter:
		{
			NewCenter = FVector(-Size.X, -Size.Y, 0);
			break;
		}
		default: break;
		}

		NewOffset = -OldCenter + NewCenter;
	}

	return NewOffset;
}

void UModelFile::RoatModel_Z90()
{
	//LocalBounds = FBox();

	for (int32 i = 0; i < m_Models.Num(); ++i)
	{
		FModel *model = m_Models[i];
		for (int32 iSection = 0; iSection < model->Sections.Num(); ++iSection)
		{
			FProcMeshSection &section = model->Sections[iSection];
			for (int32 iVert = 0; iVert < section.ProcVertexBuffer.Num(); ++iVert)
			{
				FProcMeshVertex &Vert = section.ProcVertexBuffer[iVert];
				//Vert.Normal = Vert.Normal.RotateAngleAxis(90.0f,FVector(0,0,1));
				Vert.Position = Vert.Position.RotateAngleAxis(90.0f, FVector(0, 0, 1));
				//Vert.Tangent.TangentX = Vert.Tangent.TangentX.RotateAngleAxis(90.0f, FVector(0, 0, 1));
				//LocalBounds += Vert.Position;
			}

			uint32 NumTriangle = section.ProcIndexBuffer.Num() / 3;
			for (uint32 TriIndex = 0; TriIndex < NumTriangle; ++TriIndex)
			{
				Swap<uint32>(section.ProcIndexBuffer[3 * TriIndex], section.ProcIndexBuffer[3 * TriIndex + 1]);
			}
		}
	}
	ResetSize();
	MarkDirty();
}



