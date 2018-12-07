
#pragma once

#include "ModelFile.h"
#include "SurfaceFile.h"
#include "StandardMaterialCollection.h"
#include "ProceduralMeshComponent.h"
#include "ModelPartComponent.generated.h"

UCLASS(BlueprintType)
class UModelPartComponent : public UProceduralMeshComponent
{
	GENERATED_UCLASS_BODY()
public:
	void UpdateModel(TArray<FPlane> &ClipPlanes, const FVector &Forward, float Len,  UModelFile *Model, int32 ModelIndex);
	void UpdateSurfaceMaterial(int32 iSection, UMaterialInterface *InMaterial);
	void UpdatePhysics(FConvexAggGeom &AggGeom);
	UTexture *GetTexture(TArray<FModelTexture*> &Textures, int32 TexIndex);
protected:
	UPROPERTY(Transient, BlueprintReadOnly)
	int32				ModelIndex;
	UPROPERTY(Transient, BlueprintReadOnly)
	class UModelFile	*Model;
};

