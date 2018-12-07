
#include "StandardMaterialCollection.h"

FStdMaterialInfo::FStdMaterialInfo()
	:Material(NULL)
{
}

UMaterialInterface *FStdMaterialInfo::GetMaterial()
{
	if (!Material)
	{
		UObject *MtrObj = MaterialAsset.ResolveObject();
		if (!MtrObj)
		{
			MtrObj = MaterialAsset.TryLoad();
		}
		Material = Cast<UMaterialInterface>(MtrObj);
	}
	return Material;
}

//////////////////////////////////////////////////////////////////////////
UStandardMaterialCollection::UStandardMaterialCollection(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
}

UMaterialInterface *UStandardMaterialCollection::GetMaterialByName(const FString &Name)
{
	UMaterialInterface *RetMtrl = NULL;

	for (int32 iMaterial = 0; iMaterial < StdMaterials.Num(); ++iMaterial)
	{
		FStdMaterialInfo &Slot = StdMaterials[iMaterial];
		if (Slot.GetMaterial())
		{
			FString MtrlName = Slot.GetMaterial()->GetName();
			if (MtrlName == Name)
			{
				RetMtrl = Slot.GetMaterial();
				break;
			}
		}
	}
	return RetMtrl;
}


