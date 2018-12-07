
#include "ModelActor.h"
#include "ResourceMgr.h"

AModelActor::AModelActor(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	,ModelFileComponent(NULL)
{
	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("Root"));
}

void AModelActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (ModelFileComponent)
	{
		ModelFileComponent->DestroyComponent();
		ModelFileComponent = NULL;
	}

	if (ResID.Len() > 0)
	{
		UModelFile *ModelFile = Cast<UModelFile>(UResource::LoadResInEditor(ResID));
		if (ModelFile)
		{
			if (!ModelFileComponent)
			{
				ModelFileComponent = NewObject<UModelFileComponent>(this, TEXT("ModelFileComponent"), RF_Transient);
				ModelFileComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
				ModelFileComponent->RegisterComponentWithWorld(GetWorld());
			}
			ModelFileComponent->UpdateModel(ModelFile);
		}
	}
}

UModelFile *AModelActor::GetModelFile()
{
	UModelFile *ModelFile = NULL;
	if (ModelFileComponent)
	{
		ModelFile = ModelFileComponent->GetModel();
	}
	return ModelFile;
}

FTransform AModelActor::GetModelTransform()
{
	if (ModelFileComponent)
	{
		return ModelFileComponent->GetComponentTransform();
	}
	return FTransform::Identity;
}

