#include "ProtocalTask.h"
#include "HttpModule.h"
#include "ResourceMgr.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FProtocalTask::FProtocalTask(UResource *Resource, UVaRestJsonObject *JsonObj, FProtocalDelegate &InDelegate)
{
	bFailed = false;
	bSuccessed = false;
	Delegate = InDelegate;

	if (JsonObj)
	{
		JsonStr = JsonObj->EncodeJson();
	}
}

void FProtocalTask::DoWork()
{
	if (Delegate.URL.Len() > 0 && JsonStr.Len()>0)
	{
		Request = FHttpModule::Get().CreateRequest();
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json;charset=utf-8"));
		Request->SetURL(Delegate.URL);
		Request->SetVerb(TEXT("POST"));
		Request->SetContentAsString(JsonStr);
		Request->ProcessRequest();
		Request->OnProcessRequestComplete().BindRaw(this, &FProtocalTask::OnResponseReceived);
	}
	else
	{
		bFailed = true;
	}
}

bool FProtocalTask::IsTaskDone()
{
	return bSuccessed || bFailed;
}

void FProtocalTask::ExecuteDone()
{
	if (bFailed)
	{
		Delegate.Response.ExecuteIfBound(Resource, false, NULL);
	}
}

void FProtocalTask::Clear()
{
	if (Request.IsValid())
	{
		Request->OnProcessRequestComplete().Unbind();
		Request->CancelRequest();
		Request = NULL;
	}
	Resource = NULL;
}

void FProtocalTask::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (Resource)
	{
		Collector.AddReferencedObject(Resource);
	}
}

void FProtocalTask::OnResponseReceived(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	if (HttpResponse.IsValid() && bSucceeded)
	{
		FString JsonString = HttpResponse->GetContentAsString();
		TSharedRef< TJsonReader<> > Reader = TJsonReaderFactory<>::Create(*JsonString);

		TSharedPtr<FJsonObject>	 JsonObj;
		if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
		{
			bool bRetSuccess = JsonObj->GetBoolField("success");
			TSharedPtr<FJsonObject> JsonData = JsonObj->GetObjectField("data");
			if (bRetSuccess)
			{
				bSucceeded = true;
				UVaRestJsonObject *VaJson = UVaRestJsonObject::ConstructJsonObject(NULL);
				VaJson->SetRootObject(JsonData);
				Delegate.Response.ExecuteIfBound(Resource, true, VaJson);
			}
		}
	}
}
