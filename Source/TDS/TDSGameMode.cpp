// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSGameMode.h"
#include "TDSPlayerController.h"
#include "TDSCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATDSGameMode::ATDSGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ATDSPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}