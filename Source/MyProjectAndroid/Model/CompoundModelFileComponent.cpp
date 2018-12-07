
#include "CompoundModelFileComponent.h"
#include "ModelFileComponent.h"

const static int32 EAnchorAxisX = 0x1;
const static int32 EAnchorAxisY = 0x2;
const static int32 EAnchorAxisZ = 0x4;

UCompoundModelFileComponent::UCompoundModelFileComponent(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
	, CompoundModelFile(NULL)
{
}

void UCompoundModelFileComponent::SetCompoundModelFile(UCompoundModelFile *InModelFile)
{
	DestroyChildModels();

	CompoundModelFile = InModelFile;

	if (CompoundModelFile)
	{
		int32 NumComps = CompoundModelFile->MXComponents.Num();

		ModelComponents.Empty();
		ModelComponents.AddZeroed(NumComps);

		for (int32 i = 0; i < CompoundModelFile->MXComponents.Num(); ++i)
		{
			FMXComponentSlot &Slot = CompoundModelFile->MXComponents[i];
			UModelFile *ModelFile = CompoundModelFile->GetModelFile(Slot.RefModelId);
			
			if (ModelFile)
			{
				Slot.LoadedModel = ModelFile;
				FString ResName = FString::Printf(TEXT("model_%s"), *(ModelFile->GetResourceName()));
				UModelFileComponent *Comp = NewObject<UModelFileComponent>(GetOwner(), *ResName);
				if (Comp)
				{
					Comp->UpdateModel(ModelFile);
					Comp->RegisterComponentWithWorld(GetWorld());
					ModelComponents[i] = Comp;
				}
			}
		}

		UpdateModelComponentsTransform();
	}
}

void UCompoundModelFileComponent::DestroyComponent(bool bPromoteChildren /* = false */)
{
	DestroyChildModels();

	Super::DestroyComponent(bPromoteChildren);
}

void UCompoundModelFileComponent::UpdateModelComponentsTransform()
{
	if (CompoundModelFile)
	{
		TArray<FTransform> ComponentTransforms;
		FTransform ComponentToWorldTransform = GetComponentTransform();
		
		CompoundModelFile->UpdateModelComponentTrasforms(ComponentTransforms, ComponentToWorldTransform, ModelComponents);
		UpdateWorldBounds();
	}
}

void UCompoundModelFileComponent::UpdateWorldBounds()
{
	if (CompoundModelFile)
	{
		WorldBounds.Init();

		for (int32 i = 0; i < ModelComponents.Num(); ++i)
		{
			UModelFileComponent *ModelComp = ModelComponents[i];
			if (ModelComp)
			{
				WorldBounds += ModelComp->GetWorldBounds();
			}
		}
	}
}

void UCompoundModelFileComponent::DestroyChildModels()
{
}

FTransform UCompoundModelFileComponent::GetBoneTransform(int32 SocketIndex)
{
	if (CompoundModelFile)
	{
		return CompoundModelFile->GetBoneTransform(SocketIndex) * GetComponentTransform();
	}
	return GetComponentTransform();
}

FLayoutBoneNode *UCompoundModelFileComponent::GetBone(int32 BoneIndex)
{
	if (CompoundModelFile && CompoundModelFile->Bones.IsValidIndex(BoneIndex))
	{
		return CompoundModelFile->Bones.GetData() + BoneIndex;
	}
	return NULL;
}

int32 UCompoundModelFileComponent::GetNumberOfBones()
{
	return CompoundModelFile? CompoundModelFile->Bones.Num() : 0;
}

int32 UCompoundModelFileComponent::GetNumberOfModels()
{
	return ModelComponents.Num();
}

UModelFileComponent *UCompoundModelFileComponent::GetModelFileComponent(int32 CompIndex)
{
	return ModelComponents.IsValidIndex(CompIndex) ? ModelComponents[CompIndex] : NULL;
}


