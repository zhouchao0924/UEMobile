
#include "CompoundModelFile.h"
#include "ModelFile.h"
#include "ResourceMgr.h"
#include "ModelFileComponent.h"

FMXComponentSlot::FMXComponentSlot()
	: LoadedModel(NULL)
	, ScalableX(false)
	, ScalableY(false)
	, ScalableZ(false)
	, bAchoredPositionOnly(false)
	, RefModelId(INDEX_NONE)
	, AttachedBoneIndex(INDEX_NONE)
	, AnchorTypeX(ECompPosAnchorType::EScale)
	, AnchorTypeY(ECompPosAnchorType::EScale)
	, AnchorTypeZ(ECompPosAnchorType::EScale)
{
}

FMXComponentSlot::FMXComponentSlot(int32 InModelIndex, int32 InBoneIndex, const FTransform &InRelativeTransform)
	: LoadedModel(NULL)
	, ScalableX(false)
	, ScalableY(false)
	, ScalableZ(false)
	, bAchoredPositionOnly(false)
	, RefModelId(InModelIndex)
	, AttachedBoneIndex(InBoneIndex)
	, ParentSpaceTransform(InRelativeTransform)
	, AnchorTypeX(ECompPosAnchorType::EScale)
	, AnchorTypeY(ECompPosAnchorType::EScale)
	, AnchorTypeZ(ECompPosAnchorType::EScale)
{
}

void FMXComponentSlot::Serialize(FArchive &Ar, int32 BodyVersion)
{
	if (Ar.IsSaving())
	{
		Ar << RefModelId;
		Ar << AttachedBoneIndex;
		Ar << bAchoredPositionOnly;
		Ar << AnchorTypeX;
		Ar << AnchorTypeY;
		Ar << AnchorTypeZ;
		Ar << ScalableX;
		Ar << ScalableY;
		Ar << ScalableZ;
		Ar << ParentSpaceTransform;
		ScaleAnchorPoints.BulkSerialize(Ar);
	}
	else if (Ar.IsLoading())
	{
		Ar << RefModelId;
		Ar << AttachedBoneIndex;
		Ar << bAchoredPositionOnly;
		Ar << AnchorTypeX;
		Ar << AnchorTypeY;
		Ar << AnchorTypeZ;
		Ar << ScalableX;
		Ar << ScalableY;
		Ar << ScalableZ;
		Ar << ParentSpaceTransform;
		ScaleAnchorPoints.BulkSerialize(Ar);
	}
}

void UCompoundModelFile::FHeader::Serialize(FArchive &Ar)
{
	FResourceSummary::Serialize(Ar);
}

//////////////////////////////////////////////////////////////////////////
FLayoutBoneNode::FLayoutBoneNode()
	: ParentIndex(INDEX_NONE)
	, AttachedComponentIndex(INDEX_NONE)
{
}

FLayoutBoneNode::FLayoutBoneNode(const FString &InName, int32 InParentIndex, const FTransform &InTransform)
	: Name(InName)
	, ParentIndex(InParentIndex)
	, AttachedComponentIndex(INDEX_NONE)
	, ParentSpaceTransform(InTransform)
{
}

void FLayoutBoneNode::Serialize(FArchive &Ar, int32 BodyVersion)
{
	if (Ar.IsLoading())
	{
		Ar << Name;
		Ar << ParentIndex;
		Ar << ParentSpaceTransform;
	}
	else if (Ar.IsSaving())
	{
		Ar << Name;
		Ar << ParentIndex;
		Ar << ParentSpaceTransform;
	}
}

//////////////////////////////////////////////////////////////////////////
UCompoundModelFile::UCompoundModelFile(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UCompoundModelFile::Serialize(FArchive &Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaving())
	{
		SerializeHeader(Ar);

		int32 NumComps = MXComponents.Num();
		Ar << NumComps;
		for (int32 i = 0; i < NumComps; ++i)
		{
			MXComponents[i].Serialize(Ar, m_Header.BodyVersion);
		}

		int32 NumBones = Bones.Num();
		Ar << NumBones;
		for (int32 i = 0; i < NumBones; ++i)
		{
			Bones[i].Serialize(Ar, m_Header.BodyVersion);
		}

		Ar << LocalBounds;
	}
	else if (Ar.IsLoading())
	{
		SerializeHeader(Ar);

		int32 NumComps = 0;
		Ar << NumComps;
		MXComponents.SetNum(NumComps);
		for (int32 i = 0; i < NumComps; ++i)
		{
			MXComponents[i].Serialize(Ar, m_Header.BodyVersion);
		}

		int32 NumBones = 0;
		Ar << NumBones;
		Bones.SetNum(NumBones);
		for (int32 i = 0; i < NumBones; ++i)
		{
			Bones[i].Serialize(Ar, m_Header.BodyVersion);
		}

		Ar << LocalBounds;
	}
}

int32 UCompoundModelFile::GetNumerOfModels()
{
	return Models.Num();
}

UModelFile *UCompoundModelFile::GetModelFile(int32 ModelIndex)
{
	UModelFile *ModelFile = NULL;

	if (Models.IsValidIndex(ModelIndex))
	{
		ModelFile = Models[ModelIndex];
	}

	return ModelFile;
}

bool UCompoundModelFile::IsScalableX(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		return Slot.ScalableX;
	}
	return false;
}

bool UCompoundModelFile::IsScalableY(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		return Slot.ScalableY;
	}
	return false;
}

bool UCompoundModelFile::IsScalableZ(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		return Slot.ScalableZ;
	}
	return false;
}

bool UCompoundModelFile::IsAnchorPositionOnly(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		return Slot.bAchoredPositionOnly;
	}
	return false;
}

void UCompoundModelFile::SetScalableX(int32 SlotIndex, bool bScalable)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		Slot.ScalableX = bScalable;
		MarkDirty();
	}
}

void UCompoundModelFile::SetScalableY(int32 SlotIndex, bool bScalable)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		Slot.ScalableY = bScalable;
		MarkDirty();
	}
}

void UCompoundModelFile::SetScalableZ(int32 SlotIndex, bool bScalable)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		Slot.ScalableZ = bScalable;
		MarkDirty();
	}
}

ECompPosAnchorType UCompoundModelFile::GetPosAnchorTypeX(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		return MXComponents[SlotIndex].AnchorTypeX;
	}
	return ECompPosAnchorType::EScale;
}

void UCompoundModelFile::SetPosAnchorTypeX(int32 SlotIndex, ECompPosAnchorType InAxisType)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		MXComponents[SlotIndex].AnchorTypeX = InAxisType;
	}
}

ECompPosAnchorType UCompoundModelFile::GetPosAnchorTypeY(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		return MXComponents[SlotIndex].AnchorTypeY;
	}
	return ECompPosAnchorType::EScale;
}

void UCompoundModelFile::SetPosAnchorTypeY(int32 SlotIndex, ECompPosAnchorType InAxisType)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		MXComponents[SlotIndex].AnchorTypeY = InAxisType;
	}
}

ECompPosAnchorType UCompoundModelFile::GetPosAnchorTypeZ(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		return MXComponents[SlotIndex].AnchorTypeZ;
	}
	return ECompPosAnchorType::EScale;
}

void UCompoundModelFile::SetPosAnchorTypeZ(int32 SlotIndex, ECompPosAnchorType InAxisType)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		MXComponents[SlotIndex].AnchorTypeZ = InAxisType;
	}
}

void UCompoundModelFile::SetAnchorPositionOnly(int32 SlotIndex, bool bAnchorPositionOnly)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		Slot.bAchoredPositionOnly = bAnchorPositionOnly;
		MarkDirty();
	}
}

UModelFile *UCompoundModelFile::GetModelByComponentSlot(int32 SlotIndex)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		if (Models.IsValidIndex(Slot.RefModelId))
		{
			return Models[Slot.RefModelId];
		}
	}
	return NULL;
}

void UCompoundModelFile::OnPostLoad()
{
	Super::OnPostLoad();

	if (ResMgr)
	{
		int32 Num = m_Header.Dependences.Num();
		Models.SetNum(Num);

		for (int32 i = 0; i < m_Header.Dependences.Num(); ++i)
		{
			FString ResID = m_Header.Dependences[i];
			UModelFile *Model = Cast<UModelFile>(ResMgr->FindRes(ResID));		
			Models[i] = Model;
		}

		for (int32 i = 0; i < MXComponents.Num(); ++i)
		{
			FMXComponentSlot &Slot = MXComponents[i];
			FLayoutBoneNode  &Bone = Bones[Slot.AttachedBoneIndex];
			Slot.LoadedModel = Models[Slot.RefModelId];
			Bone.AttachedComponentIndex = i;
		}
	}
}

int32 UCompoundModelFile::FindBone(const FString &Name)
{
	int32 FoundIndex = INDEX_NONE;
	for (int32 i = 0; i < Bones.Num(); ++i)
	{
		if (Bones[i].Name == Name)
		{
			FoundIndex = i;
			break;
		}
	}
	return FoundIndex;
}

void UCompoundModelFile::AddModel(UModelFile *Model, const FString &ParentBoneName, const FTransform &RelativeTransform)
{
	if (Model)
	{
		FString ResID = Model->GetResID();
		
		int32 FoundIndex = m_Header.Dependences.Find(ResID);
		if (FoundIndex == INDEX_NONE)
		{
			FoundIndex = m_Header.Dependences.Add(ResID);
		}

		int32 ParentIndex = FindBone(ParentBoneName);
		MXComponents.Emplace(FMXComponentSlot(FoundIndex, ParentIndex, RelativeTransform));

		MarkDirty(true);
	}
}

void UCompoundModelFile::SetSlotModel(int32 SlotIndex, UModelFile *InModelFile)
{
	if (MXComponents.IsValidIndex(SlotIndex))
	{
		FMXComponentSlot &Slot = MXComponents[SlotIndex];
		if (InModelFile)
		{
			FString ResID = InModelFile->GetResID();
			int32 FoundIndex = m_Header.Dependences.Find(ResID);
			if (FoundIndex == INDEX_NONE)
			{
				FoundIndex = m_Header.Dependences.Add(ResID);
			}
			Slot.RefModelId = FoundIndex;
		}
		else
		{
			Slot.RefModelId = INDEX_NONE;
			TArray<FString> NewDepends;
			for (int32 i = 0; i < MXComponents.Num(); ++i)
			{
				FMXComponentSlot &CurSlot = MXComponents[i];
				if (m_Header.Dependences.IsValidIndex(CurSlot.RefModelId))
				{
					CurSlot.RefModelId = NewDepends.AddUnique(m_Header.Dependences[CurSlot.RefModelId]);
				}
				else
				{
					CurSlot.RefModelId = INDEX_NONE;
				}
			}
			m_Header.Dependences = NewDepends;
		}

		UpdateBounds();		
		MarkDirty(true);
	}
}

void UCompoundModelFile::AddBone(const FString &Name, int32 ParentIndex, const FTransform &RelativeTransform)
{
	Bones.Emplace(FLayoutBoneNode(Name, ParentIndex, RelativeTransform));
}

FTransform UCompoundModelFile::GetBoneTransform(int32 BoneIndex)
{
	if (Bones.IsValidIndex(BoneIndex))
	{
		if (BoneTransforms.Num() != Bones.Num())
		{
			UpdateBoneTransforms();
		}

		return BoneTransforms[BoneIndex];
	}
	return FTransform::Identity;
}

void UCompoundModelFile::UpdateBoneTransforms()
{
	int32 NumBone = Bones.Num();
	BoneTransforms.SetNum(NumBone);

	for (int32 i = 0; i < NumBone; ++i)
	{
		FLayoutBoneNode &Bone = Bones[i];
		
		if (i == 1)
		{
			Bone.ParentSpaceTransform.SetRotation(FQuat::Identity);
		}

		if (Bone.ParentIndex != INDEX_NONE)
		{
			check(Bone.ParentIndex < i);
			BoneTransforms[i] = Bone.ParentSpaceTransform * BoneTransforms[Bone.ParentIndex];
		}
		else
		{
			BoneTransforms[i] = Bone.ParentSpaceTransform;
		}
	}
}

int32 UCompoundModelFile::GetParentBone(int32 BoneIndex)
{
	FLayoutBoneNode &Bone = Bones[BoneIndex];
	return Bone.ParentIndex;
}

void UCompoundModelFile::UpdateModelComponentTrasforms(TArray<FTransform> &OutTransforms, const FTransform &ComponentWorldTransform, const TArray<UModelFileComponent *> &ModelComponents)
{
	int32 NumComponent = MXComponents.Num();
	OutTransforms.SetNum(NumComponent);

	for (int32 i = 0; i < MXComponents.Num(); ++i)
	{
		FMXComponentSlot &SlotInfo = MXComponents[i];

		FTransform LocalToComponentSpace, LocalToWorldSpace;
		if (SlotInfo.AttachedBoneIndex != INDEX_NONE)
		{
			LocalToComponentSpace = SlotInfo.ParentSpaceTransform * GetBoneTransform(SlotInfo.AttachedBoneIndex);
		}
		else
		{
			LocalToComponentSpace = SlotInfo.ParentSpaceTransform;
		}

		FVector Scale = ComponentWorldTransform.GetScale3D();
		FTransform ComponentWorldTransformNoScale = FTransform(ComponentWorldTransform.ToMatrixNoScale());

		FTransform WorldTransform;
		if (SlotInfo.AttachedBoneIndex != INDEX_NONE)
		{
			WorldTransform = SlotInfo.ParentSpaceTransform * GetBoneTransform(SlotInfo.AttachedBoneIndex) * ComponentWorldTransform;
		}
		else
		{
			WorldTransform = SlotInfo.ParentSpaceTransform * ComponentWorldTransform;
		}

		FVector XAxis(1.0f, 0, 0), YAxis(0, 1.0f, 0), ZAxis(0, 0, 1.0f);
		FTransform InverseWorldTransform = WorldTransform.Inverse();

		FVector XAxisLocal = InverseWorldTransform.TransformVectorNoScale(XAxis);
		FVector YAxisLocal = InverseWorldTransform.TransformVectorNoScale(YAxis);
		FVector ZAxisLocal = InverseWorldTransform.TransformVectorNoScale(ZAxis);

		FVector InverseScale = FVector(1.0f / Scale.X, 1.0f / Scale.Y, 1.0f / Scale.Z);
		FTransform InverseScaleTransform;

		FVector WorldPosition = WorldTransform.GetLocation();
		int32 ParentBoneIndex = GetParentBone(SlotInfo.AttachedBoneIndex);

		if (Bones.IsValidIndex(ParentBoneIndex) && ModelComponents.Num()>0)
		{
			FLayoutBoneNode  &ParentBone = Bones[ParentBoneIndex];
			if (ModelComponents.IsValidIndex(ParentBone.AttachedComponentIndex))
			{
				UModelFile *ModelFile = GetModelByComponentSlot(ParentBone.AttachedComponentIndex);
				UModelFileComponent *ParentComponent = ModelComponents[ParentBone.AttachedComponentIndex];
				if (ModelFile && ParentComponent)
				{
					FBox  ParentBounds = ParentComponent->GetWorldBounds();
					FLayoutBoneNode &AttachedBone = Bones[SlotInfo.AttachedBoneIndex];

					FVector  LocalPostion = AttachedBone.ParentSpaceTransform.GetLocation();
					FBox	 LocalBox = ModelFile->GetLocalBounds();
					FVector  NoScaledWorldPosition = WorldTransform.TransformVector(LocalPostion);
					FBox	 NoScaledWorldBox = LocalBox.TransformBy(WorldTransform);

					switch (SlotInfo.AnchorTypeX)
					{
					case ECompPosAnchorType::ENegative: WorldPosition.X = ParentBounds.Min.X + (NoScaledWorldPosition.X - NoScaledWorldBox.Min.X);
					case ECompPosAnchorType::EPositive: WorldPosition.X = ParentBounds.Max.X - (NoScaledWorldBox.Max.X - NoScaledWorldPosition.X);
					default: break;
					}

					switch (SlotInfo.AnchorTypeY)
					{
					case ECompPosAnchorType::ENegative: WorldPosition.Y = ParentBounds.Min.Y + (NoScaledWorldPosition.Y - NoScaledWorldBox.Min.Y);
					case ECompPosAnchorType::EPositive: WorldPosition.Y = ParentBounds.Max.Y - (NoScaledWorldBox.Max.Y - NoScaledWorldPosition.Y);
					default: break;
					}

					switch (SlotInfo.AnchorTypeZ)
					{
					case ECompPosAnchorType::ENegative: WorldPosition.Z = ParentBounds.Min.Z + (NoScaledWorldPosition.Z - NoScaledWorldBox.Min.Z);
					case ECompPosAnchorType::EPositive: WorldPosition.Z = ParentBounds.Max.Z - (NoScaledWorldBox.Max.Z - NoScaledWorldPosition.Z);
					default: break;
					}
				}
			}
		}

		FVector LocalScale(0);

		LocalScale += XAxisLocal * ((SlotInfo.ScalableX) ? Scale.X : 1.0f);
		LocalScale += YAxisLocal * ((SlotInfo.ScalableY) ? Scale.Y : 1.0f);
		LocalScale += ZAxisLocal * ((SlotInfo.ScalableZ) ? Scale.Z : 1.0f);

		FRotator Rotation = WorldTransform.Rotator();
		LocalToWorldSpace = FTransform(FScaleMatrix(LocalScale))*FTransform(Rotation, WorldPosition, FVector(1.0f));

		if (ModelComponents.IsValidIndex(i))
		{
			UModelFileComponent *ModelComp = ModelComponents[i];
			if (ModelComp)
			{
				ModelComp->SetWorldTransform(LocalToWorldSpace);
			}
		}

		OutTransforms[i] = LocalToWorldSpace;
	}
}

int32 UCompoundModelFile::GetAttachedComponentSlotIndex(int32 AttachedBoneIndex)
{
	int32 CompSlotIndex = INDEX_NONE;

	for (int32 i = 0; i < MXComponents.Num(); ++i)
	{
		FMXComponentSlot &Slot = MXComponents[i];
		if (Slot.AttachedBoneIndex == AttachedBoneIndex)
		{
			CompSlotIndex = i;
			break;
		}
	}

	return CompSlotIndex;
}

void UCompoundModelFile::UpdateBounds()
{
	if (ResMgr)
	{
		LocalBounds.Init();

		TArray<FTransform> ModelTransforms;
		UpdateModelComponentTrasforms(ModelTransforms, FTransform::Identity, TArray<UModelFileComponent *>());

		for (int32 i = 0; i < MXComponents.Num(); ++i)
		{
			FMXComponentSlot &Slot = MXComponents[i];
			if (m_Header.Dependences.IsValidIndex(Slot.RefModelId))
			{
				FString ResID = m_Header.Dependences[Slot.RefModelId];
				UModelFile *Model = Cast<UModelFile>(ResMgr->FindRes(ResID, true));
				if (Model)
				{
					Model->ForceLoad();
					FBox ModelBox = Model->GetLocalBounds();
					ModelBox = ModelBox.TransformBy(ModelTransforms[i]);
					LocalBounds += ModelBox;
				}
			}
		}
	}
}


