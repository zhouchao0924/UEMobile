#pragma once

#include "Engine/GameInstance.h"
#include "StandardMaterialCollection.h"
#include "EditorGameInstance.generated.h"

class UProtocalImpl;
class UStandardMaterialCollection;
class UResourceMgr;

USTRUCT(BlueprintType)
struct FDRGameContext
{
	GENERATED_BODY()

	FDRGameContext()
		:ResManager(NULL),
		Protocal(NULL)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR")
	TSubclassOf<class UStandardMaterialCollection> MaterialCollection;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DR")
	TSubclassOf<class UProtocalImpl>	ProtocalClass;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR")
	UResourceMgr						*ResManager;
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "DR")
	UProtocalImpl						*Protocal;
};


UCLASS()
class MYPROJECTANDROID_API UCEditorGameInstance : public UGameInstance
{
	GENERATED_UCLASS_BODY()

	public:
		void Init() override;
		void Shutdown() override;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Config)
		FDRGameContext		Context;	
};


