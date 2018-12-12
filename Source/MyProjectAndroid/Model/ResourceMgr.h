
#pragma once

#include "Engine/EngineTypes.h"
#include "Materials/MaterialInterface.h"
#include "ResourceItem.h"
#include "StandardMaterialCollection.h"
#include "UpdateFileSystemTask.h"
#include "ResourceMgr.generated.h"

class UModelFile;
class USurfaceFile;
class UTextureImporter;
class UAsyncHttpFile;

DECLARE_LOG_CATEGORY_EXTERN(LogResMgr, Log, All);

USTRUCT(BlueprintType)
struct FMaterialListItem
{
	GENERATED_BODY()
	UPROPERTY(Transient, BlueprintReadOnly)
	int32				Index;
	UPROPERTY(Transient, BlueprintReadOnly)
	USurfaceFile		*Surface;
	UPROPERTY(Transient, BlueprintReadOnly)
	UMaterialInterface  *Material;
	UPROPERTY(Transient, BlueprintReadOnly)
	FString				 DisplayName;
	UPROPERTY(Transient, BlueprintReadOnly)
	UTexture			*PreviewImage;
};

USTRUCT(BlueprintType)
struct FResourceInfo
{
	GENERATED_BODY()

	FResourceInfo():Resource(NULL),ResType(EResType::EResUnknown)
	{
	}

	FResourceInfo(EResType InResType, const FString &InResID, const FString &InFilename, UResource *InResource)
		:Resource(InResource)
		,ResID(InResID)
		,ResType(InResType)
		,Filename(InFilename)
	{
	}
	UPROPERTY()
	TEnumAsByte<EResType>	ResType;
	UPROPERTY()
	FString					ResID;
	UPROPERTY()
	FString					Filename;
	UPROPERTY()
	UResource				*Resource;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResourceListChanged);

UCLASS(BlueprintType)
class UResourceMgr :public UObject
{
	GENERATED_UCLASS_BODY()
public:	
	static UResourceMgr *Create(UObject *Outer);
	static void Destroy();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta=(DisplayName="GetResourceManager", WorldContext = "WorldContextObject"))
	static UResourceMgr *Instance(UObject *WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta = (WorldContext = "WorldContextObject"))
	static UModelFile *AsyncLoadModel(const FString &InFilename, UObject *WorldContextObject, bool bNotifyIfLoaded = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta = (WorldContext = "WorldContextObject"))
	static USurfaceFile *AsyncLoadSurface(const FString &InFilename, UObject *WorldContextObject, bool bNotifyIfLoaded = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta = (WorldContext = "WorldContextObject"))
	static UResource *AsyncLoadRes(const FString &InResID, UObject *WorldContextObject, bool bNotifyIfLoaded = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta = (WorldContext = "WorldContextObject"))
	static void AsyncDownloadRes(const FString &Filename, const FString &Resid, EResType ResType, const FString &URL, const TArray<FDownloadingRes> &DependsResources, FOnDownloadTaskCompeleted Delegate, UObject *WorldContextObject, int32 SrvVersion = 0);
	
	UFUNCTION(BlueprintCallable, Category = "DR|Resource", meta = (WorldContext = "WorldContextObject"))
	static bool IsInLocalHas(const FString &filename);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	USurfaceFile *ExtractSurface(UModelFile *Model, int32 SurfaceIndex, const FSurfaceInfo &ChineseName, const FSurfaceInfo &EnglishName);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	UResource *FindRes(const FString &InResID, bool bNeedHeader = false);	

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	UResource *FindResByFilename(const FString &Filename, bool bNeedHeader = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	UModelFile *NewModel(bool bTransient = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	UCompoundModelFile *NewCompoundModel(bool bTransient = false);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	USurfaceFile *NewSurface();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void StopDownload();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void UpdateFromFileSystem();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void SaveAll();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	bool RequestSkuid(FString &OutSkuid);

	void ReleaseAll();

	UWorld *GetWorld() const override;

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void UploadResource(UResource *Resource, const FString &URL);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void QueryMaterialBrandTree(int32 UserID);
	
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void QueryMaterialById(int32 MaterialId);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void QueryMaterialByPage(int32 PageIndex, int32 PageSize, const FString &OrderBy, int32 BrandId, const FString &Name, int32 UserId);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void RemoveMaterialById(int32 MaterialId);
	
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void BeginCook(UResource *Resource, const FCookArgs &InArgs);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void CleanCache();

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void FreeResource(UResource *Resource);

	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void DeleteResource(UResource *Resource);
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	void DeleteMx(const FString filename);

	void ConditionalInit();
	UResource *LoadResByID(const FString &Resid, bool bNotifyIfLoaded);
	UResource *LoadResByFile(const FString &Filename, bool bNotifyIfLoaded);
	void DownloadRes(const FString &Filename, const FString &Resid, EResType ResType, const FString &URL, const TArray<FDownloadingRes> &DependsResources, FOnDownloadTaskCompeleted Delegate, int32 SrvVersion);
	void UpdateModelsByCategory(const FName &Level1, const FName &Level2, const FName &Level3, TArray<int32> &Resources);
	void GetResourceList(TArray<UResource *> &ResourceList);

protected:
	bool IsPreloadResFromFileSystem();
	UResource *CreateResByType(EResType ResType);
	void LoadRes(UResource *InResource);
	void AddRes(EResType ResType, const FString &ResID, const FString &Filename, UResource *Resource, bool bNotify);
	bool RenameFilename(UResource *InResource, const FString &NewFilename);
	void  LoadHeader(UResource *Resource);
	void  FreeSlot(int32 SlotIndex);
	int32 AllocSlot();
	UFUNCTION(BlueprintCallable, Category = "DR|Resource")
	UResource *GetSlot(int32 SlotIndex, bool bNeedHeader);
	UResource *Preload(const FString &Filename, bool bNeedHeader, bool bNotify);
	void FinishUpdateFile();
	void AbandonUpdateFile();
	bool IsAbandonUpdateFile();
	friend class UResource;
	friend class FUpdateFileSystemAsyncTask;
	static UResourceMgr *s_Instance;
public:
	UPROPERTY(Transient, BlueprintAssignable, Category = "DR|Resource")
	FOnResourceListChanged					ResourceListChanged;

	UPROPERTY(Transient)
	bool									bImportPreloadResource;
	//UPROPERTY(Transient)
	//UModelImporter							*Importer;
	
	bool												bTickable;	
	FCriticalSection									CriticalSection;

	UPROPERTY(Transient)
	TMap<FString, int32>					ResidResMap;
	UPROPERTY(Transient)
	TMap<FString, int32>					FilenameResMap;
	UPROPERTY(Transient)
	TArray<FResourceInfo>					PooledResource;
	UPROPERTY(Transient)
	TArray<int32>							UnusedResource;
	FThreadSafeCounter						FinishUpdateFileSystemCouter;
	FAutoDeleteAsyncTask<FUpdateFileSystemAsyncTask>	*UpdateFileSystemTask;
};

