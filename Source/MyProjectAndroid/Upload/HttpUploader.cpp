
#include "HttpUploader.h"
#include "Http.h"
#include "Base64.h"
#include "SanwuUEUtilitesPrivatePCH.h"

DEFINE_LOG_CATEGORY_STATIC(LogUploader, Warning, All);

UAJHttpUploader::UAJHttpUploader(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}



UAJHttpUploader* UAJHttpUploader::UploadFile(FString filename, FString url, int type)
{
	// TexturePath contains the local file full path
	FString TexturePath = filename;
	// file name
	int32 LastSlashPos;
	TexturePath.FindLastChar('/', LastSlashPos);
	FString FileName = TexturePath.RightChop(LastSlashPos + 1);
	UE_LOG(LogUploader, Log, TEXT("filename is %s"), *FileName);

	// get data
	TArray<uint8> UpFileRawData;
	FFileHelper::LoadFileToArray(UpFileRawData, *TexturePath);

	FString JsonStr;
	TSharedRef< TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR> > > JsonWriter = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR> >::Create(&JsonStr);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteValue(TEXT("fName"), FileName);
	JsonWriter->WriteValue(TEXT("file"), FBase64::Encode(UpFileRawData));
	JsonWriter->WriteValue(TEXT("type"), type);
	JsonWriter->WriteObjectEnd();
	// Close the writer and finalize the output such that JsonStr has what we want
	JsonWriter->Close();

	//UE_LOG(SANWUUEUTILITES_API, Log, TEXT("params is %p"), JsonStr);

	UAJHttpUploader* Uploader = NewObject<UAJHttpUploader>();

	//GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, JsonStr);//prints nothing

	Uploader->process(JsonStr,url,FileName);
	

	return Uploader;
}

void UAJHttpUploader::process(FString JsonStr,FString url,FString FileName)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json;charset=utf-8"));
	Request->SetURL(url);
	Request->SetVerb(TEXT("POST"));
	Request->SetContentAsString(JsonStr);
	Request->ProcessRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UAJHttpUploader::OnResponseReceived);
}

void UAJHttpUploader::OnResponseReceived(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	//GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Green, TEXT("response"));//prints nothing
	//UE_LOG(SANWU, Log, TEXT("response is ok"));
	FString MessageBody = "";

	// If HTTP fails client-side, this will still be called but with a NULL shared pointer!
	if (!HttpResponse.IsValid())
	{
		MessageBody = "{\"success\":\"Error: Unable to process HTTP Request!\"}";
		OnFail.Broadcast(MessageBody);
	}
	else if (EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
	{
		MessageBody = HttpResponse->GetContentAsString();
		OnSuccess.Broadcast(MessageBody);
	}
	else
	{
		MessageBody = FString::Printf(TEXT("{\"success\":\"HTTP Error: %d\"}"), HttpResponse->GetResponseCode());
		OnFail.Broadcast(MessageBody);
	}
}

/*
InUrl为上传URL
InFiles为本地文件路径的数组
Boundary为分隔符，用来分隔文件，其他地方不能出现该字符串，每个文件都以"\r\n--"+Boundary+"\r\n"开始（即BeginBoundry）
分隔符之后是文件头（FileHeader），文件头中name为文件域，这里用"file[]"，[]表示上传多个文件，如果是一个文件可去掉[]；filename为文件名，这里用本地文件名；文件头以两个换行符(\r\n\r\n)结束
文件头之后紧跟文件内容
所有文件内容都读出之后，最后以"\r\n--"+Boundary+"--\r\n"结束
*/
UAJHttpUploader* UAJHttpUploader::UploadFiles(FString url, TArray<FString> InFiles, int type)
{
	FString Boundary = "---------------------------" + FString::FromInt(FDateTime::Now().GetTicks());//分隔符  

	TArray<uint8> UploadContent;
	for (const FString& FilePath : InFiles)
	{
		TArray<uint8> ArrayContent;
		if (FFileHelper::LoadFileToArray(ArrayContent, *FilePath))
		{
			FString BeginBoundry = "\r\n--" + Boundary + "\r\n";
			UploadContent.Append((uint8*)TCHAR_TO_ANSI(*BeginBoundry), BeginBoundry.Len());

			FString FileHeader = "Content-Disposition: form-data;";//文件头  
			FileHeader.Append("name=\"file\";");
			FileHeader.Append("filename=\"" + FPaths::GetCleanFilename(FilePath) + "\"");
			FileHeader.Append("\r\nContent-Type: \r\n\r\n");
			UploadContent.Append((uint8*)TCHAR_TO_ANSI(*FileHeader), FileHeader.Len());

			UploadContent.Append(ArrayContent);
		}
	}
	FString EndBoundary = "\r\n--" + Boundary + "--\r\n";//结束符  
	UploadContent.Append((uint8*)TCHAR_TO_ANSI(*EndBoundary), EndBoundary.Len());

	UAJHttpUploader* Uploader = NewObject<UAJHttpUploader>();
	Uploader->startprocess(UploadContent, url, InFiles, Boundary);

	return Uploader;
	
}

void UAJHttpUploader::startprocess(TArray<uint8> UploadContent, FString url, TArray<FString> InFiles,FString Boundary)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetHeader(TEXT("Content-Type"), TEXT("multipart/form-data; boundary =" + Boundary));//请求头，数据格式必须是multipart/form-data  
	Request->SetURL(url);
	Request->SetVerb(TEXT("POST"));
	Request->SetContent(UploadContent);
	Request->ProcessRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UAJHttpUploader::OnResponseReceived);
}