// Fill out your copyright notice in the Description page of Project Settings.
// data model node for wall
#pragma once

#include "MaterialNode.h"
#include "WallNode.generated.h"

// Material UV
USTRUCT(Blueprintable)
struct FMatUVNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D UVScale;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector UVOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UVAngle;
};

// Corner
USTRUCT(Blueprintable)
struct FCornerNode
{
	GENERATED_BODY()

public:
	FCornerNode()
	{
		Name = FGuid::NewGuid().ToString().ToLower();
	}

	bool operator== (const FCornerNode &InCorner) const 
	{
		return Name == InCorner.Name;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D Position;
};

// wall config
USTRUCT(Blueprintable)
struct FWallNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector StartPnt;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector EndPnt;
	// Left Thick, InsideThick
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float LeftThick;
	// Right Thick, OuterThick
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RightThick;
	// UVScale正面,UV1
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D FrontUVScale;
	// UVScale侧面,UV2
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D SideUVScale;
	// UVScale后面,UV3
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector2D BackUVScale;
	// 左侧标尺
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bLeftRuler;
	// 右侧标尺
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bRightRuler;
	// 填充颜色
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FLinearColor Color;
	// Name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Height;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString PointA;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString PointB;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString WallA;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString WallB;
	// A进入非法模式
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bAF;
	// B进入非法模式
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bBF;
	// UV1Angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float FrontUVAngle;
	// UV2Angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float SideUVAngle;
	// UV3Angle
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float BackUVAngle;
	// UV1Position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector FrontUVPos;
	// UV2Position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector SideUVPos;
	// UV3Position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector BackUVPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMaterialNode> WallMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMaterialNode> AreaMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMaterialNode> TJXMat;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FSavedMaterialNode> WallMaterialsfromSaved;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FSavedMaterialNode> AreaMaterialsfromSaved;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FSavedMaterialNode> TJXMaterialsfromSaved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString StartCorner;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString EndCorner;

	// Extra Property
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D StartPos;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D EndPos;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D StartLeft;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D StartRight;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D EndLeft;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D EndRight;

	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D LocalStart;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D LocalEnd;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D LocalStartLeft;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D LocalStartRight;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D LocalEndLeft;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D LocalEndRight;
	UPROPERTY(Transient, BlueprintReadWrite)
		FVector2D Start2EndDir;
	UPROPERTY(Transient, BlueprintReadWrite)
	float WallAngle;
	UPROPERTY(Transient, BlueprintReadWrite)
	float WallLength;
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector2D WallCenter;
	UPROPERTY(Transient, BlueprintReadWrite)
	float ZPos;
};
