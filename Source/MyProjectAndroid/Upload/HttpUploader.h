// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "IHttpRequest.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HttpUploader.generated.h"

/*
upload file
*/
using namespace UF;
using namespace UP;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUploadFileDelegate, FString, Response);

UCLASS()
class  UAJHttpUploader : public UBlueprintAsyncActionBase
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "AJUpload|Http")
		static UAJHttpUploader* UploadFile(FString filename, FString url, int type);

	UFUNCTION(BlueprintCallable, Category = "AJUpload|Http")
		static UAJHttpUploader* UploadFiles(FString InUrl, TArray<FString> InFiles, int type);

	UPROPERTY(BlueprintAssignable)
		FUploadFileDelegate OnSuccess;

	UPROPERTY(BlueprintAssignable)
		FUploadFileDelegate OnFail;

private:
	void process(FString JsonStr, FString url, FString FileName);
	void startprocess(TArray<uint8> UploadContent, FString url, TArray<FString> InFiles,FString Boundary);

	FString DestinationPath;//最终路径


	void OnResponseReceived(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
};

