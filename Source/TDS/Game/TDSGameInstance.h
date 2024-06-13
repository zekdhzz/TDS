#pragma once


#include "Engine/DataTable.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TDS/Structs/TDSDropStructs.h"
#include "TDS/Structs/TDSWeaponStructs.h"
#include "TDSGameInstance.generated.h"

UCLASS()
class TDS_API UTDSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
 
public:
	//table
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " WeaponSetting ")
	UDataTable* WeaponInfoTable = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " WeaponSetting ")
	UDataTable* DropItemInfoTable = nullptr;
	UFUNCTION(BlueprintCallable)
	bool GetWeaponInfoByName(FName NameWeapon, FWeaponInfo& OutInfo) const;
	UFUNCTION(BlueprintCallable)
	bool GetDropItemInfoByName(FName NameItem, FDropItem& OutInfo) const;
};
