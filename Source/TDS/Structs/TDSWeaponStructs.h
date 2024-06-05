#pragma once

#include "CoreMinimal.h"
#include "TDSProjectileStructs.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TDSWeaponStructs.generated.h"

USTRUCT(BlueprintType)
struct FWeaponInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	TSubclassOf<class AWeaponDefault> WeaponClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	float WeaponDamage = 20.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	float RateOfFire = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	FProjectileInfo ProjectileSetting;
};


UCLASS()
class TDS_API UTDSWeaponStructs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};