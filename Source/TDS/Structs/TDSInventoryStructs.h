#pragma once
#include "TDSWeaponStructs.h"
#include "TDS/Enumerations/TDSWeaponEnums.h"
#include "TDSInventoryStructs.generated.h"

USTRUCT(BlueprintType)
struct FWeaponSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlot")
	FName NameItem;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponSlot")
	FAdditionalWeaponInfo AdditionalInfo;
};

USTRUCT(BlueprintType)
struct FAmmoSlot
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoSlot")
	EWeaponType WeaponType = EWeaponType::RifleType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoSlot")
	int32 Cout = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoSlot")
	int32 MaxCout = 100;
};

UCLASS()
class TDS_API UTDSInventoryStructs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};