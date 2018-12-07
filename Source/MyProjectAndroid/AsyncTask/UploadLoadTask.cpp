#include "UploadLoadTask.h"
#include "ResourceMgr.h"
#include "HttpUploader.h"
#include "Misc/Base64.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"

FUploadAsyncTask::FUploadAsyncTask(UResource *Resource, const FString &InURL)
{
	bFailed = false;
	bCommitted = false;
	bUploadFinished = false;

	if (Resource)
	{
		PendingResource = Resource;

		TArray<UResource *> Stacks;
		Stacks.Add(PendingResource);

		if (!Resource->IsLoaded())
		{
			Resource->ForceLoad();
		}

		UResourceMgr *ResMgr = Resource->ResMgr;

		if (ResMgr && Resource->IsLoaded())
		{
			while (Stacks.Num() > 0)
			{
				UResource *CurRes = Stacks.Pop();
				PendLoadingResources.Add(CurRes);

				FResourceSummary *Summary = CurRes->GetSummary();
				if (Summary)
				{
					for (int32 i = 0; i < Summary->Dependences.Num(); ++i)
					{
						UResource *SubRes = ResMgr->FindRes(Summary->Dependences[i]);
						if (SubRes)
						{
							Stacks.Add(SubRes);
						}
					}
				}
			}

			for (int32 i = 1; i< PendLoadingResources.Num(); ++i)
			{
				UResource *DepResource = PendLoadingResources[i];
				if (DepResource && DepResource->GetLocalVersion() == 0)
				{
					PendingResource->OnNotify.Broadcast(PendingResource, EResNotify::EUploadErr_DependsNotAllUploaded);
					PendLoadingResources.Empty();
					break;
				}
			}

			if (PendingResource && PendLoadingResources.Num()>0)
			{
				PendingResource->SetState(EResState::EResUploading);
			}
		}
		else
		{
			Resource->OnNotify.Broadcast(PendingResource, EResNotify::ELoadErr_LoadedFailed);
		}

		URL = InURL;
	}
}

FUploadAsyncTask::FUploadAsyncTask(UResource * Resource, const FString & InURL, int32 adduserid)
{
	AddUserId= adduserid;
	FUploadAsyncTask(Resource, InURL);
}

void FUploadAsyncTask::DoWork()
{
	if (PendingResource && PendLoadingResources.Num()>0)
	{
		PendingResource->BeginUpload();

		FString Filename = FPaths::GetCleanFilename(PendingResource->Filename);

		// get data
		TArray<uint8> RawData;
		FFileHelper::LoadFileToArray(RawData, *PendingResource->Filename);

		FString JsonStr;
		TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR> > > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR> >::Create(&JsonStr);
		JsonWriter->WriteObjectStart();
		JsonWriter->WriteValue(TEXT("fName"), Filename);
		JsonWriter->WriteValue(TEXT("file"), FBase64::Encode(RawData));
		JsonWriter->WriteValue(TEXT("type"), 8);
		JsonWriter->WriteObjectEnd();
		JsonWriter->Close();


		Request = FHttpModule::Get().CreateRequest();
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json;charset=utf-8"));
		Request->SetURL(URL);
		Request->SetVerb(TEXT("POST"));
		Request->SetContentAsString(JsonStr);
		Request->ProcessRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &FUploadAsyncTask::OnResponseReceived);
	}	
}

bool FUploadAsyncTask::IsTaskDone()
{
	return bCommitted || bFailed;
}

void FUploadAsyncTask::OnResponseReceived(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	FString JsonString;

	if (!bUploadFinished)
	{
		bool bUploadSuccess = false;

		if (PendingResource)
		{
			if (bSucceeded)
			{
				check(PendingResource);
				JsonString = HttpResponse->GetContentAsString();
				TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(*JsonString);

				if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
				{
					bool bRetSuccess = JsonObj->GetBoolField("success");
					const TSharedPtr<FJsonObject> &JsonData = JsonObj->GetObjectField("data");

					if (bRetSuccess  && JsonData->TryGetStringField("fileUrl", PendingResource->URL))
					{
						bUploadSuccess = true;
					}
					else
					{
						PendingResource->OnNotify.Broadcast(PendingResource, EResNotify::EUploadErr_UploadedJsonErr);
					}
				}
				else
				{
					if (PendingResource)
					{
						PendingResource->OnNotify.Broadcast(PendingResource, EResNotify::EUploadErr_UploadedJsonErr);
					}
				}
			}
			else
			{
				PendingResource->OnNotify.Broadcast(PendingResource, EResNotify::EUploadErr_Uploaded);
			}

			if (bUploadSuccess)
			{
				UProtocalImpl *Protocal = UProtocalImpl::GetProtocal(PendingResource->ResMgr);
				if (Protocal)
				{
					UVaRestJsonObject *VaJson = UVaRestJsonObject::ConstructJsonObject(PendingResource->ResMgr);
					Protocal->Protocal_Upload(PendingResource, VaJson, Delegate);
					if (Delegate.URL.Len() > 0)
					{
						Request = FHttpModule::Get().CreateRequest();
						Request->SetHeader(TEXT("Content-Type"), TEXT("application/json;charset=utf-8"));
						Request->SetURL(Delegate.URL);
						Request->SetVerb(TEXT("POST"));
						JsonString = VaJson->EncodeJson();
						Request->SetContentAsString(JsonString);
						Request->ProcessRequest();
						Request->OnProcessRequestComplete().BindRaw(this, &FUploadAsyncTask::OnResponseReceived);
					}
				}
			}
		}

		if (bUploadSuccess)
		{
			bUploadFinished = true;
		}
		else
		{
			bFailed = true;
			PendingResource->EndUpload(false);
		}
	}
	else
	{
		bool bCommitSuccess = false;
		
		if (bSucceeded && HttpResponse.IsValid())
		{
			JsonString = HttpResponse->GetContentAsString();

			TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(*JsonString);
			if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
			{
				bool bRetSuccess = JsonObj->GetBoolField("success");
				TSharedPtr<FJsonObject> JsonData = JsonObj->GetObjectField("data");

				if (bRetSuccess && JsonData.IsValid())
				{
					bCommitSuccess = true;
					UVaRestJsonObject *VaJson = UVaRestJsonObject::ConstructJsonObject(PendingResource->ResMgr);
					VaJson->SetRootObject(JsonData);
					
					int32 materialId = JsonData->GetIntegerField("userMaterialId");
					if (PendingResource)
					{
						PendingResource->Id = materialId;
					}

					Delegate.Response.ExecuteIfBound(PendingResource, true, VaJson);
					PendingResource->EndUpload(false);
				}
			}
		}

		if (!bCommitSuccess)
		{
			bFailed = true;
			Delegate.Response.ExecuteIfBound(PendingResource, false, NULL);
			PendingResource->EndUpload(false);
		}
	}
}

void FUploadAsyncTask::ExecuteDone()
{
	if (PendingResource)
	{
		PendingResource->SetState(EResState::EResLoaded);
		PendingResource->MarkDirty(false);
		PendingResource->OnNotify.Broadcast(PendingResource, EResNotify::EUploadOk_UploadedFinished);
	}
}

void FUploadAsyncTask::Clear()
{
	if (Request.IsValid())
	{
		Request->OnProcessRequestComplete().Unbind();
		Request->CancelRequest();
		Request = NULL;
	}
	PendingResource = NULL;
	PendLoadingResources.Empty();
}

void FUploadAsyncTask::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PendingResource);
	Collector.AddReferencedObjects(PendLoadingResources);
}


