// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VaRestJsonObject.h"
#include "Protocal.generated.h"

class UResource;
class UCEditorGameInstance;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnResponse, UResource *, Resource, bool, bSuccess, UVaRestJsonObject *, JsonObj);

USTRUCT(BlueprintType)
struct FProtocalDelegate
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FOnResponse  Response;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString		 URL;
};

UCLASS(Blueprintable)
class UProtocalImpl :public UObject
{
	GENERATED_UCLASS_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "DR|Protocal", meta=(WorldContext="WorldContextObject"))
	static UProtocalImpl *GetProtocal(UObject *WorldContextObject);

	UFUNCTION(BlueprintImplementableEvent, Category = "DR|Protocal")
	void Protocal_Upload(UResource *Resource, UVaRestJsonObject * JsonObj, FProtocalDelegate &Delegate);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "DR|Protocal")
	void Protocal_Update(UResource *Resource, UVaRestJsonObject * JsonObj, FProtocalDelegate &Delegate);

	UFUNCTION(BlueprintImplementableEvent, Category = "DR|Protocal")
	void Protocal_QueryMaterialBrandTree(int32 UserID, UVaRestJsonObject * JsonObj, FProtocalDelegate &Delegate);

	UFUNCTION(BlueprintImplementableEvent, Category = "DR|Protocal")
	void Protocal_QueryMaterialById(int32 MaterialId, UVaRestJsonObject * JsonObj, FProtocalDelegate &Delegate);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "DR|Protocal")
	void Protocal_QueryMaterialByPage(int32 PageIndex, int32 PageSize, const FString &OrderBy, int32 BrandId, const FString &Name, int32 UserId, UVaRestJsonObject * JsonObj, FProtocalDelegate &Delegate);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "DR|Protocal")
	void Protocal_RemoveMaterialById(int32 MaterialId, UVaRestJsonObject * JsonObj, FProtocalDelegate &Delegate);

	UFUNCTION(BlueprintCallable, Category = "DR|Protocal")
	void CallProtocal(UVaRestJsonObject * JsonObj, FProtocalDelegate Delegate);
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Protocal")
	FString						URL;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR|Protocal")
	UCEditorGameInstance		*MyGame;
};

