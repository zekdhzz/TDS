#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TDSCharacterEnums.generated.h"

UENUM(BlueprintType)
enum class ECharacterMovementState : uint8
{
	Aim_State UMETA(DisplayName = "Aim State"),
	AimWalk_State UMETA(DisplayName = "AimWalk State"),
	Walk_State UMETA(DisplayName = "Walk State"),
	Run_State UMETA(DisplayName = "Run State"),
	Sprint_State UMETA(DisplayName = "Sprint State")
};

UCLASS()
class TDS_API UTDSCharacterEnums : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
};
