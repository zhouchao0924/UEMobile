
#include "Resource.h"
#include "ModelFile.h"
#include "SurfaceFile.h"
#include "ResourceMgr.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

FResourceSummary::FResourceSummary()
	: bCompressed(0)
	, HeadVersion(INDEX_NONE)
	, ResType(EResUnknown)
	, LocalVersion(0)
	, ModifyVersion(0)
{

}

void FResourceSummary::Serialize(FArchive &Ar)
{
	HeadVersion = GetHeadVer();

	if (Ar.IsLoading() || Ar.IsSaving())
	{
		Ar << HeadVersion;
		Ar << ResID;
		Ar << LocalVersion;
		Ar << bCompressed;
		Ar << ResourceName;

		//ver2 <--
		if (Ar.IsLoading() && HeadVersion <= RESOURCE_HEADERVER_2)
		{
			int32	SizeX, SizeY, SizeZ;
			FString Brand, Subfamily, CategoryName, Desc;
			TEnumAsByte<EPerfPosition>	PerfPosition;
			bool	bUsePhysics;
			Ar << Brand;
			Ar << Subfamily;
			Ar << CategoryName;
			Ar << PerfPosition;
			Ar << bUsePhysics;
			Ar << SizeX;
			Ar << SizeY;
			Ar << SizeZ;
			Ar << Desc;
		}

		if ((Ar.IsLoading() && HeadVersion > RESOURCE_HEADERVER_2) || Ar.IsSaving())
		{
			Ar << ModifyVersion;
		}
	}

	if (Ar.IsSaving())
	{
		int32 NumDeps = Dependences.Num();
		Ar << NumDeps;
		for (int32 i = 0; i < Dependences.Num(); ++i)
		{
			Ar << Dependences[i];
		}
	}
	else if (Ar.IsLoading())
	{
		int32 NumDeps = 0;
		Ar << NumDeps;
		Dependences.SetNum(NumDeps);
		for (int32 i = 0; i < Dependences.Num(); ++i)
		{
			Ar << Dependences[i];
		}
	}

	BodyVersion = GetCodeVer();
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		Ar << BodyVersion;
	}
}

//////////////////////////////////////////////////////////////////////////
UResource::UResource(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
	,bDirty(0)
	,bLoaded(0)
	,bHeadLoaded(0)
	,PreviewImage(NULL)
	,CurrentSrvVersion(0)
	,ResState(EResState::EResPending)
	,Id(0)
{
	EResType ResType = GetResType();
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		Summary->ResType = ResType;
	}
}

void UResource::MarkDirty(bool bModifyed)
{
	bDirty = 1;

	if (bModifyed)
	{
		if (OnNotify.IsBound())
		{
			OnNotify.Broadcast(this, EResNotify::EModified);
		}
		IncModifyVersion();
	}
}

void UResource::OnDownloaded()
{
	ForceLoad();
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		ResetModifyVersion();
		Save();
	}
}

FString UResource::GetResID()
{
	FResourceSummary *Summary = GetSummary();
	return  Summary? Summary->ResID : TEXT("");
}

void UResource::SetResID(const FString &InResID)
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		Summary->ResID = InResID;
	}
	MarkDirty(false);
}

const TCHAR *GetResTypeName(EResType ResType)
{
	switch (ResType)
	{
	case EResModel: return TEXT("mx");
	case EResSurface: return TEXT("sx");
	case EResCompoundModel: return TEXT("cmx");
	default: break;
	}
	return TEXT("invalid");
}

static FString FingerMark = TEXT("1*&343!2)*Yds;Qd^`}/?3,");

void UResource::SaveMark(FArchive &Ar)
{
	if (Ar.IsSaving())
	{
		Ar << FingerMark;
	}
}

bool UResource::CheckMark(FArchive &Ar)
{
	if (Ar.IsLoading())
	{
		FString Mark;
		Ar << Mark;
		if (Mark.Compare(FingerMark) == 0)
		{
			return true;
		}
	}
	return false;
}

bool UResource::SkipMark(FArchive &Ar)
{
	if (Ar.IsLoading())
	{
		TEnumAsByte<EResType> resType;
		Ar << resType;
		return CheckMark(Ar);
	}
	return false;
}

FString UResource::GetUserSaveFilename(const FString &ResourceName, const FString &Skuid, EResType ResType, bool IsArtManage)
{
	FString Path = "";
	if (IsArtManage) {
		Path = FPaths::ProjectContentDir() + TEXT("_Basic/Materials/BaseSx/");
	}
	else {
		Path = FPaths::ProjectSavedDir() + TEXT("Resources/Local/");
	}
	FString Ext = GetResTypeName(ResType);
	return FString::Printf(TEXT("%s%s_%s.%s"), *Path, *ResourceName, *Skuid, *Ext);
}

void UResource::Save()
{
	if (bLoaded)
	{
		TEnumAsByte<EResType> ResType = GetResType();

		FArchive *Writer = IFileManager::Get().CreateFileWriter(*Filename);

		if (Writer != NULL)
		{
			*Writer << ResType;
			SaveMark(*Writer);

			Serialize(*Writer);
			Writer->Close();
			ClearDirty();
		}
	}
}

bool UResource::Load()
{
	if (ResMgr)
	{
		ResMgr->LoadRes(this);
		return true;
	}

	return false;
}

void UResource::Free()
{
	if (ResMgr)
	{
		ResMgr->FreeResource(this);
	}
}

UWorld *UResource::GetWorld() const
{
	return ResMgr ? ResMgr->GetWorld() : nullptr;
}

FString UResource::GetResourceName()
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		return Summary->ResourceName;
	}
	return TEXT("");
}

bool UResource::SetResourceName(const FString &Name, bool IsArtManage)
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		FString NewFilename = UResource::GetUserSaveFilename(Name, GetResID(), GetResType(), IsArtManage);
		if (ResMgr && ResMgr->RenameFilename(this, NewFilename))
		{
			Summary->ResourceName = Name;
			MarkDirty();
			return true;
		}		
	}
	return false;
}

void UResource::GetPreviewImageSize(int32 &OutSizeX, int32 &OutSizeY)
{
	FResourceSummary *Header = GetSummary();
	if (Header)
	{
		OutSizeX = Header->Image.SizeX;
		OutSizeY = Header->Image.SizeY;
	}
}

int32 UResource::GetServerVersion()
{
	if (DRInfo)
	{
		int32 SrvVersion = FGenericPlatformMath::CeilToInt(DRInfo->GetNumberField(TEXT("version")));
		return SrvVersion;
	}
	return 0;
}

void UResource::SetServerVersion(int32 Version)
{
	if (DRInfo)
	{
		DRInfo->SetNumberField(TEXT("version"), Version);
	}
}

void UResource::IncModifyVersion()
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		Summary->ModifyVersion++;
	}
}

int32 UResource::GetLocalVersion()
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		return  Summary->LocalVersion;
	}
	return 0;
}

int32 UResource::GetModifyVersion()
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		return  Summary->ModifyVersion;
	}
	return 0;
}

void UResource::ResetModifyVersion()
{
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		Summary->ModifyVersion = Summary->LocalVersion;
		MarkDirty(false);
	}
}

bool UResource::IsNeedUpgrade(int32 SrvVersion/*= 0*/)
{
	if (SrvVersion == 0)
	{
		SrvVersion = GetServerVersion();
	}
	return SrvVersion > GetLocalVersion();
}

bool UResource::IsModified()
{
	return GetModifyVersion() != GetLocalVersion();
}

void UResource::BeginUpload()
{
	ForceLoad();
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		Summary->LocalVersion++;
		MarkDirty(false);
		Save();
	}
}

void UResource::EndUpload(bool bSuccess)
{
	ForceLoad();
	FResourceSummary *Summary = GetSummary();
	if (Summary)
	{
		if (bSuccess)
		{
			ResetModifyVersion();
			SetServerVersion(Summary->LocalVersion);
			MarkDirty(false);
		}
		else
		{
			Summary->LocalVersion--;
			MarkDirty(false);
		}		
		Save();
	}
}

void UResource::SetState(EResState NewState)
{
	ResState = NewState;

	if (StateChanged.IsBound())
	{
		StateChanged.Broadcast(this, ResState, DRInfo);
	}
}

void UResource::SerializeHeader(FArchive &Ar)
{
	if (Ar.IsSaving() || Ar.IsLoading())
	{
		Ar << Id;
		Ar << URL;
		FResourceSummary *Header = GetSummary();
		if (Header)
		{
			Header->Serialize(Ar);
		}

		FString JsonStr = "";

		if (Ar.IsLoading())
		{
			if (Header)
			{
				if (Header->HeadVersion > RESOURCE_HEADERVER_1)
				{
					Ar << JsonStr;
					if (JsonStr.Len() > 0)
					{
						DRInfo = UVaRestJsonObject::ConstructJsonObject(this);
						DRInfo->DecodeJson(JsonStr);
					}
				}
			}
		}
		else if (Ar.IsSaving())
		{
			if (DRInfo != NULL)
			{
				JsonStr = DRInfo->EncodeJson();
			}
			Ar << JsonStr;
		}
	}
}

void UResource::Serialize(FArchive &Ar)
{

}

void UResource::ForceLoad()
{
	if (!bLoaded)
	{
		FResourceSummary *Summary = GetSummary();
		if (Summary)
		{
			for (int32 i = 0; i < Summary->Dependences.Num(); ++i)
			{
				FString Urid = Summary->Dependences[i];
				UResource *DepRes = ResMgr->FindRes(Urid);
				if (DepRes)
				{
					DepRes->ForceLoad();
				}
			}
		}

		FArchive *Writer = IFileManager::Get().CreateFileReader(*Filename);
		if (Writer && UResource::SkipMark(*Writer))
		{
			Serialize(*Writer);
			bLoaded = 1;
			bHeadLoaded = 1;
			OnPostLoad();
			Writer->Close();
		}
	}
}

void UResource::Unload()
{
	bLoaded = 0;
	bHeadLoaded = 0;
}

void UResource::SetDRInfo(UVaRestJsonObject *InDRInfo)
{
	DRInfo = InDRInfo;

	if (StateChanged.IsBound())
	{
		StateChanged.Broadcast(this, ResState, DRInfo);
	}
	
	MarkDirty(false);
}

void UResource::CheckResource()
{
	MarkDirty(false);
}

void UResource::GetDependencyResources(TArray<FString> &OutDepResources)
{
	OutDepResources.Empty();

	const TArray<FString> *pResources = GetDepends();
	if (pResources)
	{
		OutDepResources = *pResources;
	}
}

UResourceMgr *EditorResMgr = NULL;

UResource *UResource::LoadResInEditor(const FString &ResID)
{
	UResource *Resource = NULL;

	if (!EditorResMgr)
	{
		EditorResMgr = NewObject<UResourceMgr>();
		EditorResMgr->AddToRoot();
		EditorResMgr->ConditionalInit();
	}

	if (EditorResMgr)
	{
		Resource = EditorResMgr->FindRes(ResID);
		if (Resource)
		{
			Resource->ForceLoad();
		}
	}

	return Resource;
}

void UResource::Clone(UResource *Other)
{
	if (Other && (Other->GetClass() == GetClass()))
	{
		TArray<uint8> Data;
		FMemoryWriter ArWriter(Data);
		Other->Serialize(ArWriter);

		FMemoryReader ArReader(Data);
		Clean();
		Serialize(ArReader);

		MarkDirty(true);
	}
}


