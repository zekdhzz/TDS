#pragma once
#include "TDSInventoryStructs.h"
#include "Engine/DataTable.h"
#include "TDSDropStructs.generated.h"

USTRUCT(BlueprintType)
struct FDropItem : public FTableRowBase
{
	GENERATED_BODY()

	///Index Slot by Index Array
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropWeapon")
	UStaticMesh* WeaponStaticMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropWeapon")
	USkeletalMesh* WeaponSkeletalMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DropWeapon")
	FWeaponSlot WeaponInfo;
};

UCLASS()
class TDS_API UTDSDropStructs : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
