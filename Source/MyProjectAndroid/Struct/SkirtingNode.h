#pragma once

#include "WallNode.h"
#include "CgTypes.h"
#include "SkirtingNode.generated.h"


UENUM(BlueprintType)
enum class ESkirtingType : uint8 
{
	SkirtingLine = 1,
	SkirtingTopLine,
};

UENUM(BlueprintType)
enum class ESkirtingSrcType : uint8
{
	SrcFormDrawing,
	SrcFromSyncPath,
};


UENUM(BlueprintType)
enum class ESkirtingSnapType : uint8
{
	SnapNull,
	SnapHead = 1,
	SnapTail,
};

struct FSkirtingInterval
{
public:
	FSkirtingInterval(const FVector2D &InCacheStart, const FVector2D &InCacheEnd)
		:IntervalStartCache(InCacheStart), IntervalEndCache(InCacheEnd)
	{
	}

	const FVector2D& GetStart() const
	{
		return IntervalStartCache;
	}

	const FVector2D& GetEnd() const
	{
		return IntervalEndCache;
	}

protected:
	FVector2D IntervalStartCache;
	FVector2D IntervalEndCache;
};


USTRUCT(Blueprintable)
struct  FSkirtingSegNode
{
	GENERATED_BODY()
public:
	FSkirtingSegNode()
	{
		ID = FGuid::NewGuid().ToString().ToLower();

		SkirtingMeshModelID = MaterialModelID = -1;
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCornerNode		SkirtingStartCorner;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FCornerNode		SkirtingEndCorner;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString			ID;
// properties in skirting node, used for temp
	UPROPERTY(Transient, BlueprintReadWrite)
	float			StartSectionAngle;
	//UPROPERTY(Transient, BlueprintReadWrite)
	//FVector2D		StartSectionDir;
	UPROPERTY(Transient, BlueprintReadWrite)
	float			EndSectionAngle;
	//UPROPERTY(Transient, BlueprintReadWrite)
	//FVector2D		EndSectionDir;
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector StartClippedPlaneDir;
	UPROPERTY(Transient, BlueprintReadWrite)
	FVector EndClippedPlaneDir;

	UPROPERTY(Transient, BlueprintReadWrite)
	FString SkirtingMeshID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkirtingMeshModelID;
	UPROPERTY(Transient, BlueprintReadWrite)
	FString MaterialID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialModelID;

	UPROPERTY(Transient, BlueprintReadWrite)
	ESkirtingType SkirtingType;
	UPROPERTY(Transient, BlueprintReadWrite)
	FString	OwnerWall; // may be null

	void AddInterval(const FVector2D &StartPnt, const FVector2D &EndPnt)
	{
		SkirtingIntervals.Add(FSkirtingInterval(StartPnt, EndPnt));
	}

	void ClearIntervalCache()
	{
		SkirtingIntervals.Empty();
	}

	const TArray<FSkirtingInterval>& GetCachedIntervals() const
	{
		return SkirtingIntervals;
	}

protected:
	TArray<FSkirtingInterval> SkirtingIntervals;
};


// skirting node
USTRUCT(Blueprintable)
struct FSkirtingNode
{
	GENERATED_BODY()
public:
	FSkirtingNode()
	{
		SkirtingMeshModelID = MaterialModelID = -1;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkirtingMeshModelID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SkirtingMeshID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialModelID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MaterialID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		ECgHeadingDirection HeadingDir;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkirtingType SkirtingType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkirtingSrcType SrcType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClosed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkirtingSegNode> SkirtingSegments;
};


// skirting path surrounding area
USTRUCT(Blueprintable)
struct FRoomPath
{
	GENERATED_BODY()

	FRoomPath()
	{
		bIsLivingOrDiningRoom = false;
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector2D> InnerRoomPath;
	UPROPERTY(BlueprintReadWrite)
	bool bIsLivingOrDiningRoom;
};


