
#pragma once

#include "Resource.h"
#include "ResourceItem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResAsyncLoad, const FString &, Skuid, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResStateChanged, const FString &, Skuid);

USTRUCT(BlueprintType)
struct FResourceItem
{
	GENERATED_USTRUCT_BODY() 

	FResourceItem();
	FResourceItem(UResource *InResource);
public:
	UPROPERTY(Transient, BlueprintAssignable)
	FOnResAsyncLoad		OnAsyncLoad;

	UPROPERTY(Transient, BlueprintAssignable)
	FOnResStateChanged  OnStateChanged;

	UPROPERTY(EditAnywhere)
	FString  Skuid;

	UPROPERTY(EditAnywhere)
	UResource *Resource;
	
	UPROPERTY(EditAnywhere)
	uint32	 bLoaded:1;

	UPROPERTY(EditAnywhere)
	uint32	 bLoading : 1;
	
	UPROPERTY(EditAnywhere)
	uint32	 bDepLoading : 1;

	UPROPERTY(EditAnywhere)
	uint32	 bCorrupt : 1;

public:
	uint32	bHasChekUpdated : 1;
	uint32  bNeedUpdate : 1;
};

USTRUCT(BlueprintType)
struct FSurfaceInfo
{
	GENERATED_BODY()
	UPROPERTY(Transient, BlueprintReadWrite)
	FString Name;
};

USTRUCT(BlueprintType)
struct FDownloadingRes
{
	GENERATED_BODY()

	FDownloadingRes() { Resource = NULL; HttpFile = NULL; }

	FDownloadingRes(EResType InType, const FString &InURL, const FString &InFilename)
		:URL(InURL)
		, ResType(InType)
		, Filename(InFilename)
		, Resource(NULL)
		, HttpFile(NULL)
		, bNeedDownload(false)
	{
	}

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EResType> ResType;

	UPROPERTY(EditAnywhere)
	FString  URL;

	UPROPERTY(EditAnywhere)
	FString  Filename;

	UPROPERTY()
	UResource *Resource;

	UPROPERTY()
	class UAsyncHttpFile *HttpFile;

	UPROPERTY()
	bool bNeedDownload;
};

USTRUCT(BlueprintType)
struct FCookArgs
{
	GENERATED_BODY()

	FCookArgs()
		:bCheckCollision(0)
		,bAdjustCenter(0)
	{
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bCheckCollision : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bAdjustCenter : 1;
};

UENUM(BlueprintType)
enum class ECenterAdjustType :uint8
{
	Original,

	BottomCenter,
	TopCenter,
	LeftCenter,
	RightCenter,
	FrontCenter,
	BackCenter,

	TopFrontEdgeCenter,
	TopBackEdgeCenter,
	TopLeftEdgeCenter,
	TopRightEdgeCenter,

	BottomFrontEdgeCenter,
	BottomBackEdgeCenter,
	BottomLeftEdgeCenter,
	BottomRightEdgeCenter,

	LeftFrontEdgeCenter,
	RightFrontEdgeCenter,
	LeftBackEdgeCenter,
	RightBackEdgeCenter
};

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDownloadTaskCompeleted, UResource *, Resource, float, Progress, bool, bFinished);

