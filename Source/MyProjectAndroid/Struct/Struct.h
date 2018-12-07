// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#pragma warning(disable: 4146)

#include "VaRestJsonObject.h"
#include "GameFramework/Actor.h"
#include "WallNode.h"
#include "MaterialNode.h"
#include "Struct.generated.h"

USTRUCT(BlueprintType)
//MX模型依赖的材质结构体
struct FMatOfMX
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString					ResID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int					    ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString					URL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int						MtrlChannelId;
};

// 模型UV记录结构体
USTRUCT(BlueprintType)
struct FUVOffset
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = Geometry)
		float	U;

	UPROPERTY(BlueprintReadWrite, Category = Geometry)
		float	V;
};

USTRUCT(BlueprintType)
struct FUVScale
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = Geometry)
		float	U;

	UPROPERTY(BlueprintReadWrite, Category = Geometry)
		float	V;
};

USTRUCT(BlueprintType)
//MX模型保存的结构体
struct FMXSaved
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor>     ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString					ResID;  

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int					    ModelId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform				Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool					HasDeleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMatOfMX>		DependsMat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUVOffset	UVOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUVScale	UVScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float		UVRot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool ApplyShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoomClassID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CraftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32					SkuId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString	ModelTag;
};

//TODO--用于墙体，地面，踢脚线等硬装所使用材质的SX文件存储结构体（不存储模型的材质，模型材质由FMXSaved结构体中SX的Resid关联）


// ZCBlueprintFunctionLibrary移过来的，不知道是干什么用的
USTRUCT(BlueprintType)
struct FMXFileLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ZHUtil")
		FString      ResID;

	UPROPERTY(BlueprintReadOnly, Category = "ZHUtil")
		TArray<int>  ResIndexInResIDList;
};

// 用于存储CSV数据
USTRUCT(BlueprintType)
struct FCSVAtom
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = CSV)
		TArray<FString> Content;

public:

	FCSVAtom()
	{
		Content.Init(TEXT(""), 0);
	};

	FCSVAtom(int32 Number)
	{
		Content.Init(TEXT(""), Number);
	};

	void Init(int32 Number)
	{
		Content.Init(TEXT(""), Number);
	}
};

USTRUCT(BlueprintType)
struct FCSVFile
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, Category = CSV)
		int32 Cols;

	UPROPERTY(BlueprintReadOnly, Category = CSV)
		FCSVAtom Header;

	UPROPERTY(BlueprintReadOnly, Category = CSV)
		TArray<FCSVAtom> Content;

public:

	FCSVFile()
	{
		Cols = 0;

		Header.Init(0);

		Content.Init(0, 0);
	};

	FCSVFile(int32 InCols)
	{
		Cols = InCols;

		Header.Init(Cols);

		Content.Init(Cols, 0);
	};

	void SetHeader(FCSVAtom InHeader)
	{
		if (InHeader.Content.Num() >= Cols)
		{
			for (int i = 0; i < Cols; i++)
			{
				Header.Content[i] = InHeader.Content[i];
			}
		}
		else
		{
			for (int i = 0; i < Cols; i++)
			{
				if (i < InHeader.Content.Num())
				{
					Header.Content[i] = InHeader.Content[i];
				}
				else
				{
					Header.Content[i] = TEXT("");
				}
			}
		}
	};

	void AddContent(FCSVAtom InContent)
	{
		FCSVAtom TempAtom(Cols);

		if (InContent.Content.Num() >= Cols)
		{
			for (int i = 0; i < Cols; i++)
			{
				TempAtom.Content[i] = InContent.Content[i];
			}
		}
		else
		{

			for (int i = 0; i < InContent.Content.Num(); i++)
			{
				TempAtom.Content[i] = InContent.Content[i];
			}
		}

		Content.Add(TempAtom);
	};
};

// 方案迁移页面

UENUM(BlueprintType)
enum class EItemType : uint8
{
	IT_Unkown		UMETA(DisplayName = "未知"),
	IT_Normal		UMETA(DisplayName = "成品"),
	IT_Customized	UMETA(Displayname = "定制"),
	IT_Gift			UMETA(DisplayName = "赠品"),
};

USTRUCT(BlueprintType)
struct FRoomUsage
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomUsage)
		int32	Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomUsage)
		FString Name;

	FRoomUsage() {};

	FRoomUsage(int32 InId, FString InName);

};

USTRUCT(BlueprintType)
struct FRoomMark
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomMark)
		int32	Mark;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomMark)
		FString	Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomMark)
		float	Size;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = RoomMark)
		TArray<FRoomUsage>	Usages;

	FRoomMark() {};

	FRoomMark(int32 InMark, FString InName, float InSize, TArray<FRoomUsage> InUsages);
};

//选配包子空间结构体
USTRUCT(BlueprintType)
struct FMXChildDesignSaved
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 ParentId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 RoomId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 UsageId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 DesignId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 DesignIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		FString RoomName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		TArray<FMXSaved> MxActorList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		FString AreaTag;
};

//选配包主空间结构体
USTRUCT(BlueprintType)
struct FMXMainDesignSaved
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 RoomId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 UsageId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 DesignId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		int32 DesignIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		FString RoomName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		TArray<FMXSaved> MxActorList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		TArray<FMXChildDesignSaved> MxDesignList;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MXDesignSaved)
		FString AreaTag;
};

//全景图点位信息

USTRUCT(BlueprintType)
struct FPanoramaPoint
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		FString PointName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		TArray<FString> ConnectPoint;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		FVector PointLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		FRotator PointRotation;
};

//全景图Room信息

USTRUCT(BlueprintType)
struct FPanormaRoom
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		int RoomID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		TArray<FPanoramaPoint> PointList;
};

//全景图方案信息

USTRUCT(BlueprintType)
struct FPanormaSolution
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		int SolutionID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		FString url;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Panorama)
		TArray<FPanormaRoom> RoomList;
};


// 权限系统 - 界面 子结构体

USTRUCT(BlueprintType)
struct FAuthorityCell
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Authority)
		FString		AuthorityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Authority)
		bool		bVisibility;
};


// 权限系统 - 界面 分类主结构体

USTRUCT(BlueprintType)
struct FAuthority
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Authority)
		FString		CategoryName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Authority)
		bool		bVisibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Authority)
		TArray<FAuthorityCell> Child;
};


USTRUCT(BlueprintType)
struct FComputerInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		FString		CPU;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		FString		GPU;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		FString		OS;

	// 内存容量信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		FString		RAM;
};

USTRUCT(BlueprintType)
struct FModelStatus
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		int Type;//状态 1.移动 2.创建 3.删除
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		int Id;//模型Id
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		bool NoAreaModel;//是否是无区域模型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		AActor *Actor;//
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		FMXSaved MxSaved;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ComputerInfo)
		FRotator rotationXYZ;
};


// Http error codes enum
UENUM(BlueprintType)
enum class EHttpCodes : uint8
{
	Unknown,
	Continue,
	SwitchProtocol,
	Ok,
	Created,
	Accepted,
	Partial,
	NoContent,
	ResetContent,
	PartialContent,
	Ambiguous,
	Moved,
	Redirect,
	RedirectMethod,
	NotModified,
	UseProxy,
	RedirectKeepVerb,
	BadRequest,
	Denied,
	PaymentReq,
	Forbidden,
	NotFound,
	BadMethod,
	NoneAcceptable,
	ProxyAuthReq,
	RequestTimeout,
	Conflict,
	Gone,
	LengthRequired,
	PrecondFailed,
	RequestTooLarge,
	UriTooLong,
	UnsupportedMedia,
	TooManyRequests,
	RetryWith,
	ServerError,
	NotSupported,
	BadGateway,
	ServiceUnavail,
	GatewayTimeout,
	VersionNotSup,
};


USTRUCT(BlueprintType)
struct FHardDesignNode
{
	GENERATED_USTRUCT_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	ModelID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	SkuID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	RoomClassID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	CraftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	DesignID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	UsageID = -1;
};

USTRUCT(BlueprintType)
struct FModelDesignNode
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	ModelID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	SkuID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	RoomClassID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	CraftID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	DesignID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	UsageID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	MaterialModelID= -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DesignNode")
		int32	MaterialSKUID = -1;
};

// 某一面墙依赖关系
USTRUCT(BlueprintType)
struct FWallDependsArea
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		int32	RoomID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		int32	ChannelID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString WallGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString	Acreage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		TArray<FHardDesignNode>	Designs;
};

// 某一区域依赖关系

USTRUCT(BlueprintType)
struct FAreaDepends
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32	RoomID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString	GroundAcreage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString	RoofAcreage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString	GroundPerimeter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString	RoofPerimeter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FHardDesignNode>	GroundDesigns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FHardDesignNode>	RoofDesigns;

};

//窗的依赖关系
USTRUCT(BlueprintType)
struct FWindowDependsArea
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32	RoomID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString WindowGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString	WindowArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FModelDesignNode> WindowDesigns;
};

//门的依赖关系
USTRUCT(BlueprintType)
struct FDoorDependsArea
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32	RoomID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString DoorGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString	DoorArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FModelDesignNode> DoorDesigns;
};

//门套的依赖关系
USTRUCT(BlueprintType)
struct FDoorPocketDependsArea
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32	RoomID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString DoorPocketGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString DoorPocketLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FModelDesignNode> DoorPocketDesigns;
};

//过门石依赖关系
USTRUCT(BlueprintType)
struct FThresholdDependsArea
{
	GENERATED_USTRUCT_BODY() 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32 RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString ThresholdGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString ThresholdLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FHardDesignNode> ThresholdDesigns;
};

//踢脚线依赖关系
USTRUCT(BlueprintType)
struct FSkirtingDependsArea
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32 RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WallDependency")
		FString SkirtingGUID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString SkirtingLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FModelDesignNode> SkirtingDesigns;
};

//硬装模型依赖关系
USTRUCT(BlueprintType)
struct FHardRiftModelDependsArea
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		int32 RoomID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		FString Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AreaDependency")
		TArray<FModelDesignNode> HardRiftModelDesigns;
};

USTRUCT(BlueprintType)
struct FMXDWSaved
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<AActor>     ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool					HasDeleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool					ApplyShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32>			ModelIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32>			SkulIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32>			RoomClassId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32>			CraftId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString					Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString					TagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FTransform				Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMaterialNode>	DependsMat;
};

USTRUCT(BlueprintType)
struct FHardCraftStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		int32					CraftId;//项目Id

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		FString					CraftName;//项目名称

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		FString					Image;	//工艺图片

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		int32					baseFlag;//是不是默认工艺
};

USTRUCT(BlueprintType)
struct FHardClassStruct
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		int32					ClassId;//项目Id

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		FString					ClassName;//项目名称

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		TArray<FHardCraftStruct>	CraftList;
};


USTRUCT(BlueprintType)
struct FHardState
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		int32					roomUsageId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HardCraft)
		TArray<FHardClassStruct>	ClassList;

};



//Area材质多空间使用
USTRUCT(BlueprintType)
struct FMaterialArrayOfDesign
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMaterialNode> MaterialNodeArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DesignID;

};


//门窗多空间使用
USTRUCT(BlueprintType)
struct FDoorAndWindowOfDesign
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMXDWSaved> DWSavedArray;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<int32> IDs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 DesignID;
	
};

//物体隐藏属性
USTRUCT(BlueprintType)
struct FHiddenItemsInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int			modelId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int			index;
};

//窗户信息
USTRUCT(BlueprintType)
struct FWindowAreaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector			WindowPos;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector			WindowDir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D		WindowSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float			WinodwIntensity;
};

USTRUCT(BlueprintType)
struct FAutoSaveStruct
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool			IsEnableAutoSave = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32			TimeInterval = 600;

};

USTRUCT(BlueprintType)
struct FGroupActorInfo
{
	GENERATED_USTRUCT_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString				GroupName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform			GroupActorTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString				GroupSelfTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString>		GroupChildTag;
};

USTRUCT(BlueprintType)
struct FDnaRoomSaved
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 RoomId;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<UVaRestJsonObject*>	ModelJsonList;
};