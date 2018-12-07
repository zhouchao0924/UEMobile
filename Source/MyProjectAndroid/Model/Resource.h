
#pragma once

#include "VaRestJsonObject.h"
#include "Engine/DataTable.h"
#include "Resource.generated.h"

typedef FString FSkuid;

#define  RESOURCE_HEADERVER_1	1			//->2	jason DRInfo
#define  RESOURCE_HEADERVER_2	2			//->3	Header 
#define  RESOURCE_HEADERVER		3

UENUM(BlueprintType)
enum EResType
{
	EResSurface,
	EResModel,
	EResScene,
	EResCompoundModel,
	EResUnknown
};

UENUM(BlueprintType)
enum EPerfPosition
{
	EPosNone = 0,
	EPosGround = 0x1,
	EPosCeil = 0x2,
	EPosWall = 0x4,
	EPosDesktop = 0x8,
};	

extern const TCHAR *GetResTypeName(EResType ResType);

USTRUCT(BlueprintType)
struct FSubCategory
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Resource")
	FName					Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Resource")
	TArray<FName>			LeafCategories;
};

USTRUCT(BlueprintType)
struct FResCategoryData :public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(Transient, BlueprintReadOnly)
	FName KeyName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FSubCategory> SubCategories;
};


USTRUCT(BlueprintType)
struct FResourceSummary
{
	GENERATED_BODY()

	FResourceSummary();
	
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	TEnumAsByte<EResType>		ResType;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	int32						HeadVersion;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	FString						ResID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Resource")
	FString						ResourceName;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	int32						LocalVersion;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	int32						ModifyVersion;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	uint8						bCompressed;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	TArray<FString>				Dependences;
	UPROPERTY(BlueprintReadOnly, Category = "DR|Resource")
	int32						BodyVersion;

	struct {
		int32			SizeX;
		int32			SizeY;
		TArray<uint8>	Data;
	} Image;

	virtual void  Serialize(FArchive &Ar);
	virtual int32 GetHeadVer() { return RESOURCE_HEADERVER; }
	virtual int32 GetCodeVer() { return 0; }
};

UENUM(BlueprintType)
enum class EResState : uint8
{
	EResFailed,
	EResPending,
	EResLoaded,
	EResUploading,
	EResDownloading,
};

UENUM(BlueprintType)
enum class EResNotify :uint8
{
	ECooked,
	EModified,
	EUploadErr_DependsNotAllUploaded,
	EUploadErr_Uploaded,
	EUploadErr_UploadedJsonErr,
	EUploadOk_UploadedFinished,
	ELoadErr_LoadedFailed,
};

class UResourceMgr;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceNotify, UResource *, Resource, EResNotify, Notify);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnResourceStateChanged, UResource *, Resource, EResState, State, UVaRestJsonObject *, DRInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourcePreviewImageChanged, UResource *, Resource, UTexture *, Texture);

UCLASS(BlueprintType)
class UResource :public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta=(WorldContext="WorldContextObject"))
	bool Load();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void Save();
	
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void Free();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	static FString GetUserSaveFilename(const FString &ResourceName, const FString &Skuid, EResType ResType,bool IsArtManage=false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	virtual EResType GetResType() { return EResUnknown; }

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	FString GetResourceName();
	
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	bool  SetResourceName(const FString &Name, bool IsArtManage = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	FString GetResID();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void SetResID(const FString &InResID);

	UFUNCTION(BlueprintPure, Category = "DR|Resource")
	int32 GetServerVersion();

	UFUNCTION(BlueprintPure, Category = "DR|Resource")
	int32 GetLocalVersion();

	UFUNCTION(BlueprintPure, Category = "DR|Resource")
	int32 GetModifyVersion();

	UFUNCTION(BlueprintPure, Category = "DR|Resource")
	bool  IsNeedUpgrade(int32 SrvVersion = 0);

	UFUNCTION(BlueprintPure, Category = "DR|Resource")
	bool IsModified();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void BeginUpload();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void EndUpload(bool bSuccess);

	UFUNCTION(BlueprintCallable, Category = "DR|Surface", meta = (WorldContext = "WorldContextObject"))
	void ForceLoad();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void SetDRInfo(UVaRestJsonObject *InDRInfo);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	virtual void CheckResource();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void SetURL(const FString &InURL) { URL = InURL; }

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void GetDependencyResources(TArray<FString> &OutDepResources);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	static UResource *LoadResInEditor(const FString &ResID);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void Clone(UResource *Other);

	virtual void MarkDirty(bool bModifyed = true);
	virtual void ClearDirty() { bDirty = 0; }
	virtual bool IsDirty() { return bDirty != 0; }
	virtual void OnDownloaded();
	void IncModifyVersion();

	virtual void Serialize(FArchive &Ar);
	virtual void SerializeHeader(FArchive &Ar);
	virtual FResourceSummary *GetSummary() { return NULL; }
	virtual void OnPostLoad() { PendLoadingRefCount.Reset(); bLoaded = 1; }
	virtual const TArray<FString> *GetDepends() { return NULL; }
	void GetPreviewImageSize(int32 &OutSizeX, int32 &OutSizeY);
	virtual void Clean() { }

	bool IsNeedLoad() { return !IsPendingLoad() && !bLoaded; }
	bool IsLoaded() { return bLoaded; }
	bool IsHeadLoaded() { return bHeadLoaded; }
	static void SaveMark(FArchive &Ar);
	static bool CheckMark(FArchive &Ar);
	static bool SkipMark(FArchive &Ar);
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void SetState(EResState NewState);
	void SetPendingLoad() { PendLoadingRefCount.Set(1); }
	bool IsPendingLoad() { return PendLoadingRefCount.GetValue() > 0; }
	virtual void Unload();
	void SetServerVersion(int32 Version);
	void ResetModifyVersion();

	UWorld *GetWorld() const override;
public:
	UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "DR|Resource")
	FOnResourceStateChanged				StateChanged;
	UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "DR|Resource")
	FOnResourceNotify					OnNotify;
	UPROPERTY(EditAnywhere, BlueprintAssignable, Category = "DR|Resource")
	FOnResourcePreviewImageChanged		PreviewImageChanged;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Resource")
	FString								CategoryName;
public:
	uint32								bDirty : 1;
	FThreadSafeCounter					PendLoadingRefCount;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	FString								Filename;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	int32								Id;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	FString								URL;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	uint32								bHeadLoaded : 1;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	uint32								bLoaded:1;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	UResourceMgr						*ResMgr;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	EResState							ResState;
	UPROPERTY(Transient)
	UTexture							*PreviewImage;
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	int32								CurrentSrvVersion;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR|Resource")
	UVaRestJsonObject					*DRInfo;
};

