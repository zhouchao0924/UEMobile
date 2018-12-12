
#include "ModelPartComponent.h"
#include "PhysicsEngine/BodySetup.h"

#pragma warning(disable : 4668)

UModelPartComponent::UModelPartComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	ModelIndex = INDEX_NONE;
	bUseComplexAsSimpleCollision = 0;
}

void UModelPartComponent::UpdateModel(TArray<FPlane> &ClipPlanes, const FVector &Forward, float Len, UModelFile *InModel, int32 InModelIndex)
{
	Model = InModel;
	ModelIndex = InModelIndex;

	if (Model != NULL && Model->m_Models.IsValidIndex(ModelIndex))
	{
		FModel *modelData = Model->m_Models[ModelIndex];	
		
		ClearAllMeshSections();
		
		FTransform WorldTransform = GetComponentTransform();
		FTransform WorldToLocal = WorldTransform.Inverse();
		FMatrix	   WorldToLocalTM = WorldToLocal.ToMatrixNoScale();

		for (int32 iSection = 0; iSection<modelData->Sections.Num(); ++iSection)
		{
			FProcMeshSection &Section = modelData->Sections[iSection];
			if (ClipPlanes.Num() > 0)
			{
				FProcMeshSection NewSection = Section;
				for (int iPlane = 0; iPlane < ClipPlanes.Num(); ++iPlane)
				{
					FPlane &Plane = ClipPlanes[iPlane];
					for (int iVert = 0; iVert < NewSection.ProcVertexBuffer.Num(); ++iVert)
					{
						FProcMeshVertex &Vert = NewSection.ProcVertexBuffer[iVert];
						FVector P0 = WorldTransform.TransformPosition(Vert.Position);
						float D = Plane.PlaneDot(P0);
						if (D < 0)
						{
							FVector P1;
							if ((Forward | Plane.GetSafeNormal())>0)
							{
								P1 = P0 + Forward*Len;
							}
							else
							{
								P1 = P0 - Forward*Len;
							}
							Vert.Position = WorldToLocal.TransformPosition(FMath::LinePlaneIntersection(P0, P1, Plane));
						}
					}
				}
				SetProcMeshSection(iSection, NewSection);
			}
			else
			{
				SetProcMeshSection(iSection, Section);
			}
		}

		for (int32 iSection = 0; iSection < modelData->RefMaterials.Num(); ++iSection)
		{
			int32 MaterialInedx = modelData->RefMaterials[iSection];
			UMaterialInterface *Ue4Material = Model->GetUE4Material(MaterialInedx);
			if (Ue4Material)
			{
				UpdateSurfaceMaterial(iSection, Ue4Material);
			}
		}

		SetMobility(EComponentMobility::Movable);
		SetCollisionProfileName(TEXT("BlockAll"));
		SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		UpdatePhysics(modelData->AggGeom);
	}
}

void UModelPartComponent::UpdateSurfaceMaterial(int32 iSection, UMaterialInterface *InMaterial)
{
	if (InMaterial!=NULL)
	{
		if (iSection != INDEX_NONE)
		{
			SetMaterial(iSection, InMaterial);
		}
		else
		{
			if (Model && Model->m_Models.IsValidIndex(ModelIndex))
			{
				FModel *modelData = Model->m_Models[ModelIndex];
				if (modelData)
				{
					for (int32 SectionIndex = 0; SectionIndex < modelData->RefMaterials.Num(); ++SectionIndex)
					{
						int32 MaterialInedx = modelData->RefMaterials[SectionIndex];
						SetMaterial(ModelIndex, InMaterial);
					}
				}
			}
		}
	}
}

UTexture *UModelPartComponent::GetTexture(TArray<FModelTexture*> &Textures, int32 TexIndex)
{
	if (Textures.IsValidIndex(TexIndex))
	{
		FModelTexture *texInfo = Textures[TexIndex];
		if (texInfo)
		{
			return texInfo->GetTexture();
		}
	}
	return NULL;
}

void UModelPartComponent::UpdatePhysics(FConvexAggGeom &AggGeom)
{	
	if (bPhysicsStateCreated)
	{
		DestroyPhysicsState();
	}

	bool bCreatePhysState = true;

	if (AggGeom.ConvexElems.Num() > 0)
	{
		// Ensure we have a BodySetup
		if (ProcMeshBodySetup == NULL)
		{
			// The body setup in a template needs to be public since the property is Tnstanced and thus is the archetype of the instance meaning there is a direct reference
			ProcMeshBodySetup = NewObject<UBodySetup>(this, NAME_None, (IsTemplate() ? RF_Public : RF_NoFlags));
			ProcMeshBodySetup->BodySetupGuid = FGuid::NewGuid();

			ProcMeshBodySetup->bDoubleSidedGeometry = true;
			ProcMeshBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;
		}

		// Fill in simple collision convex elements
		ProcMeshBodySetup->bGenerateMirroredCollision = true;
		ProcMeshBodySetup->AggGeom.ConvexElems = AggGeom.ConvexElems;

		// Set trace flag
		ProcMeshBodySetup->CollisionTraceFlag = bUseComplexAsSimpleCollision ? CTF_UseComplexAsSimple : CTF_UseDefault;

		// New GUID as collision has changed
		ProcMeshBodySetup->BodySetupGuid = FGuid::NewGuid();
		// Also we want cooked data for this
		ProcMeshBodySetup->bHasCookedCollisionData = true;
		
//#if WITH_RUNTIME_PHYSICS_COOKING || WITH_EDITOR
//		// Clear current mesh data
//		ProcMeshBodySetup->InvalidatePhysicsData();
//		// Create new mesh data
//		ProcMeshBodySetup->CreatePhysicsMeshes();
//#endif // WITH_RUNTIME_PHYSICS_COOKING || WITH_EDITOR

		// Create new instance state if desired
		if (bCreatePhysState)
		{
			CreatePhysicsState();
		}
	}
}

