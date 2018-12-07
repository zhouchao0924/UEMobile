
#include "ModelCommons.h"
#include "ModelFile.h"

void FChunk::Serialize(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Offset;
		Ar << Size;
	}
}

void FModel::Serialize(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving())
	{
		int32 NumMtrl = RefMaterials.Num();
		Ar << NumMtrl;
		Ar.Serialize(RefMaterials.GetData(), NumMtrl * sizeof(int32));
		
		int32 NumSections = Sections.Num();
		Ar << NumSections;
		for (int32 i = 0; i < Sections.Num(); ++i)
		{
			SerializeMeshSection(Sections[i], Ar, Ver);
		}
	}
	else if (Ar.IsLoading())
	{
		int32 NumMtrl = 0;
		Ar << NumMtrl;
		RefMaterials.SetNum(NumMtrl);
		Ar.Serialize(RefMaterials.GetData(), NumMtrl * sizeof(int32));

		int32 NumSection = 0;
		Ar << NumSection;
		Sections.SetNum(NumSection);
		for (int32 i = 0; i < Sections.Num(); ++i)
		{
			SerializeMeshSection(Sections[i], Ar, Ver);
		}
	}

	SerializeAggGeom(Ar, Ver);

	SerializeUCX(Ar, Ver);
}

void FModel::SerializeMeshSection(FProcMeshSection &Section, FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving())
	{
		int32 NumVerts = Section.ProcVertexBuffer.Num();
		Ar << NumVerts;

		for (int32 i = 0; i < NumVerts; ++i)
		{
			FProcMeshVertex &Vert = Section.ProcVertexBuffer[i];
			SerializeVertex(Vert, Ar, Ver);
		}

		int32 NumIndices = Section.ProcIndexBuffer.Num();
		Ar << NumIndices;

		Ar.Serialize(Section.ProcIndexBuffer.GetData(), sizeof(int32)*NumIndices);

		Ar << Section.SectionLocalBox;
		Ar << Section.bEnableCollision;
		Ar << Section.bSectionVisible;
	}
	else if (Ar.IsLoading())
	{
		int32 NumVerts = 0;
		Ar << NumVerts;

		Section.ProcVertexBuffer.SetNum(NumVerts);
		for (int32 i = 0; i < NumVerts; ++i)
		{
			FProcMeshVertex &Vert = Section.ProcVertexBuffer[i];
			SerializeVertex(Vert, Ar, Ver);
		}

		int32 NumIndices = 0;
		Ar << NumIndices;
		Section.ProcIndexBuffer.SetNum(NumIndices);

		Ar.Serialize(Section.ProcIndexBuffer.GetData(), sizeof(int32)*NumIndices);

		Ar << Section.SectionLocalBox;
		Ar << Section.bEnableCollision;
		Ar << Section.bSectionVisible;
	}
}

void FModel::SerializeVertex(FProcMeshVertex &Vertex, FArchive &Ar, uint32 Ver)
{
	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Vertex.Position;
		Ar << Vertex.Normal;
		Ar << Vertex.Tangent.bFlipTangentY;
		Ar << Vertex.Tangent.TangentX;
		Ar << Vertex.Color;
		Ar << Vertex.UV0;
	}
}

void FModel::SerializeUCX(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsLoading())
	{
		if (Ver > MODELFILE_BODY_VER_2)
		{
			int32 Num = 0;
			Ar << Num;
			UCXData.Vertices.SetNum(Num);
			if (Num > 0)
			{
				Ar.Serialize(UCXData.Vertices.GetData(), sizeof(FVector)*Num);
			}

			Num = 0;
			Ar << Num;
			UCXData.Indices.SetNum(Num);
			if (Num > 0)
			{
				Ar.Serialize(UCXData.Indices.GetData(), sizeof(int32)*Num);
			}
		}
	}
	else if (Ar.IsSaving())
	{
		int32 Num = 0;

		Num = UCXData.Vertices.Num();
		Ar << Num;
		if (Num > 0)
		{
			Ar.Serialize(UCXData.Vertices.GetData(), sizeof(FVector)*Num);
		}

		Num = UCXData.Indices.Num();
		Ar << Num;
		if (Num > 0)
		{
			Ar.Serialize(UCXData.Indices.GetData(), sizeof(int32)*Num);
		}
	}
}



void FModel::SerializeAggGeom(FArchive &Ar, uint32 Ver)
{
	if (Ar.IsLoading())
	{
		if (Ver > MODELFILE_BODY_VER_1)
		{
			AggGeom.Serialize(Ar);
		}
	}
	else if (Ar.IsSaving())
	{
		AggGeom.Serialize(Ar);
	}
}

void FConvexAggGeom::Serialize(FArchive &Ar)
{
	if (Ar.IsLoading())
	{
		int32 Num = 0;
		Ar << Num;
		ConvexElems.SetNum(Num);
	}
	else if (Ar.IsSaving())
	{
		int32 Num = ConvexElems.Num();
		Ar << Num;
	}

	if (Ar.IsLoading() || Ar.IsSaving())
	{
		for (int32 i = 0; i < ConvexElems.Num(); ++i)
		{
			FKConvexElem &Elem = ConvexElems[i];
			Ar << Elem.VertexData;
			Ar << Elem.ElemBox;
		}
	}
}

