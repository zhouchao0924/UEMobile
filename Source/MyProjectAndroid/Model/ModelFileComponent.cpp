
#include "ModelFileComponent.h"
#include "ResourceMgr.h"
//#include "DRGameMode.h"
//#include "EditorUtils.h"

UModelFileComponent::UModelFileComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	,AdjustType(ECenterAdjustType::Original)
{
}

void UModelFileComponent::SetModelAlignType(ECenterAdjustType InAdjustType)
{
	UModelFile *Model = GetModel();
	if (Model)
	{
		FTransform RelativeModelTransform = GetOffset(Model, InAdjustType);
		for (int32 i = 0; i < Parts.Num(); ++i)
		{
			UModelPartComponent *Comp = Parts[i];
			if (Comp)
			{
				Comp->SetRelativeTransform(RelativeModelTransform);
			}
		}
	}
	AdjustType = InAdjustType;
}

FTransform UModelFileComponent::GetOffset(UModelFile *Model, ECenterAdjustType AlignType)
{
	FVector Offset = Model->GetOffsetByCenterType(AlignType);
	return FTransform(FRotator(0, 180.0f, 0), Offset, Model->Scale3D);
}

FTransform UModelFileComponent::GetRelativeTransform(UModelFile *Model)
{
	if (AdjustType == ECenterAdjustType::Original)
	{
		return FTransform(FRotator(0, 180.0f, 0), Model->Offset, Model->Scale3D);
	}
	return  GetOffset(Model, AdjustType);
}

void UModelFileComponent::AddClipPlane(const FPlane &InPlane)
{
	ClipPlanes.Add(InPlane);
}

void UModelFileComponent::UpdateModel(UModelFile *Model)
{
	if (Model!=NULL)
	{
		for (int32 i = 0; i < Parts.Num(); ++i)
		{
			UModelPartComponent *Comp = Parts[i];
			Comp->DestroyComponent();
		}

		FTransform WorldTransform = GetComponentTransform();
		FVector Forward = WorldTransform.Rotator().Vector();

		Parts.Empty();
		FTransform RelativeModelTransform = GetRelativeTransform(Model);

		FVector Size = Model->GetLocalBounds().GetSize();

		for (int i = 0; i < Model->m_Models.Num(); ++i)
		{
			UModelPartComponent *Comp = NewObject<UModelPartComponent>(GetOwner());
			if (Comp)
			{
				Comp->RegisterComponentWithWorld(GetWorld());
				Comp->SetRelativeTransform(RelativeModelTransform);
				Comp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
				Comp->UpdateModel(ClipPlanes, Forward, Size.X, Model, i);
				Parts.Add(Comp);
			}
		}

		ModelResID = Model->GetResID();
	}
}

FBox UModelFileComponent::GetWorldBounds()
{
	FBox WorldBounds;
	FTransform WorldTransform = GetComponentTransform();
	WorldBounds = LocalBounds.TransformBy(WorldTransform);
	return WorldBounds;
}

UModelFile *UModelFileComponent::GetModel()
{
	UResourceMgr *ResMgr = UResourceMgr::Instance(this);
	UModelFile *ModelFile = Cast<UModelFile>(ResMgr ? ResMgr->FindRes(ModelResID, true) : NULL);
	if (ModelFile)
	{
		ModelFile->ForceLoad();
	}
	return ModelFile;
}

void UModelFileComponent::UpdateMaterial(int32 MaterialIndex)
{
	UModelFile *ModelFile = GetModel();
	if (ModelFile)
	{
		ModelFile->ForceLoad();
		FModelMaterial *material = ModelFile->GetMaterial(MaterialIndex);

		TArray<FMaterialSlot> materialSlots;
		ModelFile->GetMaterialInfo(MaterialIndex, materialSlots);

		for (int32 i = 0; i < materialSlots.Num(); ++i)
		{
			FMaterialSlot &slot = materialSlots[i];
			UModelPartComponent *Part = GetPart(slot.iModel);
			if (Part)
			{
				UMaterialInterface *Ue4Material = ModelFile->GetUE4Material(MaterialIndex);
				Part->UpdateSurfaceMaterial(slot.iSection, Ue4Material);
			}
		}
	}
}

void UModelFileComponent::SetMaterialByPart(int32 iPart, UMaterialInterface *Material)
{
	if (Parts.IsValidIndex(iPart))
	{
		UModelPartComponent *PartComp = Parts[iPart];
		if (PartComp)
		{
			PartComp->UpdateSurfaceMaterial(INDEX_NONE, Material);
		}
	}
}

void UModelFileComponent::SetMaterialByChannel(int32 iMtrlChannel, UMaterialInterface *Material)
{
	UModelFile *ModelFile = GetModel();
	if (ModelFile && Material)
	{
		TArray<FMaterialSlot> materialSlots;
		ModelFile->GetMaterialInfo(iMtrlChannel, materialSlots);

		for (int32 i = 0; i < materialSlots.Num(); ++i)
		{
			FMaterialSlot &slot = materialSlots[i];
			UModelPartComponent *Part = GetPart(slot.iModel);
			if (Part)
			{
				Part->UpdateSurfaceMaterial(slot.iSection, Material);
			}
		}
	}
}


UModelPartComponent *UModelFileComponent::GetPart(int32 Index)
{
	if (Parts.IsValidIndex(Index))
	{
		return Parts[Index];
	}
	return NULL;
}

void UModelFileComponent::UpdatePhysics()
{
	UModelFile *ModelFile = GetModel();
	if (ModelFile)
	{
		for (int32 i = 0; i < Parts.Num(); ++i)
		{
			UModelPartComponent *PartComp = Parts[i];
			if (ModelFile->m_Models.IsValidIndex(i))
			{
				FModel *Model = ModelFile->m_Models[i];
				PartComp->UpdatePhysics(Model->AggGeom);
			}
		}
	}
}

void UModelFileComponent::SetSimulatePhysic(bool bSimulate)
{
	for (int32 i = 0; i < Parts.Num(); ++i)
	{
		UModelPartComponent *PartComp = Parts[i];
		if (PartComp)
		{
			if (bSimulate)
			{
				PartComp->SetMobility(bSimulate ? EComponentMobility::Movable : EComponentMobility::Static);
			}
			PartComp->SetSimulatePhysics(bSimulate);
		}
	}
}

void UModelFileComponent::SetAffectByShadow(bool bAffectByShadow)
{
	for (int32 i = 0; i < Parts.Num(); ++i)
	{
		UModelPartComponent *PartComp = Parts[i];
		if (PartComp)
		{
			PartComp->SetCastShadow(bAffectByShadow);
			PartComp->bAffectDistanceFieldLighting = bAffectByShadow;
			PartComp->bAffectDynamicIndirectLighting = bAffectByShadow;
			PartComp->MarkRenderStateDirty();
		}
	}
}

void UModelFileComponent::SetCastShadow(bool bCastShadow)
{
	for (int32 i = 0; i < Parts.Num(); ++i)
	{
		UModelPartComponent *PartComp = Parts[i];
		if (PartComp)
		{
			PartComp->SetCastShadow(bCastShadow);
			PartComp->bCastFarShadow = bCastShadow;
			PartComp->bCastStaticShadow = bCastShadow;
			PartComp->bCastDynamicShadow = bCastShadow;
			PartComp->MarkRenderStateDirty();
		}
	}
}

void UModelFileComponent::DestroyComponent(bool bPromoteChildren /* = false */)
{
	for (int32 i = 0; i < Parts.Num(); ++i)
	{
		UModelPartComponent *PartComp = Parts[i];
		if (PartComp)
		{
			PartComp->DestroyComponent(bPromoteChildren);
		}
	}
	Super::DestroyComponent(bPromoteChildren);
}

