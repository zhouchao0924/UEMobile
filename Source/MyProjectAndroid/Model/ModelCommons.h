// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ModelTexture.h"
#include "ModelMaterial.h"
#include "ResourceItem.h"
//#include "TextureCompressorModule.h"
#include "ProceduralMeshComponent.h"

#define SAFE_DELETE(p)     do { if(p) { delete (p); (p) = nullptr; } } while(0)

struct FChunk
{
	uint32		Offset;
	uint32		Size;
	void Serialize(FArchive &Ar, uint32 Ver);
};

struct FConvexAggGeom
{
	TArray<FKConvexElem> ConvexElems;
	void Serialize(FArchive &Ar);
};

struct FUCXData
{
	TArray<FVector> Vertices;
	TArray<int32>   Indices;
};

struct FModel
{
	TArray<int32>			 RefMaterials;
	TArray<FProcMeshSection> Sections;
	FConvexAggGeom			 AggGeom;
	FUCXData				 UCXData;
	void Serialize(FArchive &Ar, uint32 Ver);
	void SerializeMeshSection(FProcMeshSection &Section, FArchive &Ar, uint32 Ver);
	void SerializeVertex(FProcMeshVertex &Vertex, FArchive &Ar, uint32 Ver);
	void SerializeAggGeom(FArchive &Ar, uint32 Ver);
	void SerializeUCX(FArchive &Ar, uint32 Ver);
};

