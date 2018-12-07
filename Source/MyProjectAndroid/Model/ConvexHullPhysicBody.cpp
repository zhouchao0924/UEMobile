
#include "ConvexHullPhysicBody.h"
#include "ModelCommons.h"
#include "ModelFile.h"
#include "StanHull/hull.h"

// Only enabling on windows until other platforms can test!
#define USE_VHACD  (PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_MAC)

#include "Misc/FeedbackContext.h"
#include "PhysicsEngine/ConvexElem.h"

#if USE_VHACD

#include "ThirdParty/VHACD/public/VHACD.h"

#if PLATFORM_MAC
#include <OpenCL/cl.h>
#endif

#else

#ifdef __clang__
// HACD headers use pragmas that Clang doesn't recognize (inline depth)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"	// warning : unknown pragma ignored [-Wunknown-pragmas]
#endif

#include "ThirdParty/HACD/HACD_1.0/public/HACD.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // USE_VHACD

#include "PhysicsEngine/BodySetup.h"

DEFINE_LOG_CATEGORY_STATIC(LogConvexDecompTool, Log, All);

using namespace VHACD;

class FVHACDProgressCallback : public IVHACD::IUserCallback
{
public:
	FVHACDProgressCallback(void) {}
	~FVHACDProgressCallback() {};

	void Update(const double overallProgress, const double stageProgress, const double operationProgress, const char * const stage, const char * const    operation)
	{
		FString StatusString = FString::Printf(TEXT("Processing [%s]..."), ANSI_TO_TCHAR(stage));
	};
};

void DecomposeConvexMeshToHulls(FConvexAggGeom &AggGeom, const TArray<FVector>& InVertices, const TArray<int32>& InIndices, float InAccuracy, int32 InMaxHullVerts)
{
	bool bSuccess = false;

	// Validate input by checking bounding box
	FBox VertBox(0);
	for (FVector Vert : InVertices)
	{
		VertBox += Vert;
	}

	// If box is invalid, or the largest dimension is less than 1 unit, or smallest is less than 0.1, skip trying to generate collision (V-HACD often crashes...)
	if (VertBox.IsValid == 0 || VertBox.GetSize().GetMax() < 1.f || VertBox.GetSize().GetMin() < 0.1f)
	{
		return;
	}

	FVHACDProgressCallback VHACD_Callback;

	IVHACD::Parameters VHACD_Params;
	VHACD_Params.m_resolution = 1000000; // Maximum number of voxels generated during the voxelization stage (default=100,000, range=10,000-16,000,000)
	VHACD_Params.m_maxNumVerticesPerCH = InMaxHullVerts; // Controls the maximum number of triangles per convex-hull (default=64, range=4-1024)
	VHACD_Params.m_concavity = 0.3f * (1.f - FMath::Clamp(InAccuracy, 0.f, 1.f)); // Maximum allowed concavity (default=0.0025, range=0.0-1.0)
	VHACD_Params.m_callback = &VHACD_Callback;
	VHACD_Params.m_oclAcceleration = false;

	IVHACD* InterfaceVHACD = CreateVHACD();

	const float* const Verts = (float*)InVertices.GetData();
	const unsigned int NumVerts = InVertices.Num();
	TArray<uint32_t> TrisUInt;
	for (const int32& TriIndex : InIndices)
	{
		TrisUInt.Add(TriIndex);
	}
	const uint32_t* const Tris = (uint32_t*)TrisUInt.GetData();
	const unsigned int NumTris = InIndices.Num() / 3;

	//bSuccess = InterfaceVHACD->Compute(Verts, 3, NumVerts, Tris, 3, NumTris, VHACD_Params);
	bSuccess = InterfaceVHACD->Compute(Verts, NumVerts, Tris, NumTris, VHACD_Params);

	if (bSuccess)
	{
		int32 NumHulls = InterfaceVHACD->GetNConvexHulls();

		for (int32 HullIdx = 0; HullIdx < NumHulls; HullIdx++)
		{
			// Create a new hull in the aggregate geometry
			FKConvexElem ConvexElem;

			IVHACD::ConvexHull Hull;
			InterfaceVHACD->GetConvexHull(HullIdx, Hull);
			for (uint32 VertIdx = 0; VertIdx < Hull.m_nPoints; VertIdx++)
			{
				FVector V;
				V.X = (float)(Hull.m_points[(VertIdx * 3) + 0]);
				V.Y = (float)(Hull.m_points[(VertIdx * 3) + 1]);
				V.Z = (float)(Hull.m_points[(VertIdx * 3) + 2]);

				ConvexElem.VertexData.Add(V);
			}


			ConvexElem.UpdateElemBox();
			AggGeom.ConvexElems.Add(ConvexElem);
		}
	}

	InterfaceVHACD->Clean();
	InterfaceVHACD->Release();
}

void DecomposeConvexMeshToHulls2(FConvexAggGeom &AggGeom, const TArray<FVector>& InVertices, const TArray<int32>& InIndices, float InAccuracy, int32 InMaxHullVerts)
{
	HullLibrary Library;

	int32 NumVert = InVertices.Num();

	TArray<double> DVertices;
	DVertices.SetNum(NumVert);

	double *pVertex = DVertices.GetData();
	for (int32 i = 0; i < NumVert; ++i)
	{
		pVertex[0] = InVertices[i].X;
		pVertex[1] = InVertices[i].Y;
		pVertex[2] = InVertices[i].Z;
		pVertex += 3;
	}

	HullDesc Desc;
	HullResult Result;
	Desc.mMaxVertices = 1000;
	Desc.mNormalEpsilon = 0.1f;
	Desc.mSkinWidth = 0.001f;
	Desc.mVcount = InVertices.Num();
	Desc.mVertices = DVertices.GetData();

	if (QE_OK == Library.CreateConvexHull(Desc, Result))
	{
		FKConvexElem ConvexElem;

		for (uint32 VertIdx = 0; VertIdx < Result.mNumOutputVertices; VertIdx++)
		{
			FVector V;
			V.X = Result.mOutputVertices[3 * VertIdx + 0];
			V.Y = Result.mOutputVertices[3 * VertIdx + 1];
			V.Z = Result.mOutputVertices[3 * VertIdx + 2];
			ConvexElem.VertexData.Add(V);
		}

		ConvexElem.UpdateElemBox();
		AggGeom.ConvexElems.Add(ConvexElem);
	}
}

void GenerateModelFileConvexHull(UModelFile *ModelFile, float Accuracy, int32 MaxHullVerts, bool bCleanExsit)
{
	TArray<FModel *> SubModels = ModelFile->GetSubModels();

	for (int32 i = 0; i < SubModels.Num(); ++i)
	{
		FModel *Model = SubModels[i];
		if (Model && (bCleanExsit || Model->AggGeom.ConvexElems.Num() == 0))
		{
			GenerateModelConvexHull(Model, Accuracy, MaxHullVerts);
		}
	}
}

void GenerateModelConvexHull(FModel *Model, float Accuracy, int32 MaxHullVerts)
{
	if (Model)
	{
		Model->AggGeom.ConvexElems.Empty();

		if (Model->UCXData.Indices.Num() > 0)
		{
			DecomposeConvexMeshToHulls(Model->AggGeom, Model->UCXData.Vertices, Model->UCXData.Indices, Accuracy, MaxHullVerts);
		}
		else
		{
			TArray<FVector> Vertices;

			for (int32 iSection = 0; iSection < Model->Sections.Num(); ++iSection)
			{
				FProcMeshSection &Section = Model->Sections[iSection];

				Vertices.Empty();
				for (int32 iVert = 0; iVert < Section.ProcVertexBuffer.Num(); ++iVert)
				{
					FVector Vert = Section.ProcVertexBuffer[iVert].Position;
					Vertices.Add(Vert);
				}

				TArray<int32> ProIndexBufferInt;
				for (uint32 &ProIndex : Section.ProcIndexBuffer)
				{
					ProIndexBufferInt.Add(ProIndex);
				}
				DecomposeConvexMeshToHulls(Model->AggGeom, Vertices, ProIndexBufferInt, Accuracy, MaxHullVerts);
			}
		}
	}
}


