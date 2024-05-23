// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSCharacter.h"

#include <string>

#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"

ATDSCharacter::ATDSCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(
		TEXT("Material'/Game/Enviroment/Assets/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATDSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	MovementTick();
	SoothingCameraZoom(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			if (UWorld* World = GetWorld())
			{
				FHitResult HitResult;
				FCollisionQueryParams Params(NAME_None, FCollisionQueryParams::GetUnknownStatId());
				FVector StartLocation = TopDownCameraComponent->GetComponentLocation();
				FVector EndLocation = TopDownCameraComponent->GetComponentRotation().Vector() * 2000.0f;
				Params.AddIgnoredActor(this);
				World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, Params);
				FQuat SurfaceRotation = HitResult.ImpactNormal.ToOrientationRotator().Quaternion();
				CursorToWorld->SetWorldLocationAndRotation(HitResult.Location, SurfaceRotation);
			}
		}
		else if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATDSCharacter::InputAxisX);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATDSCharacter::InputAxisY);
	PlayerInputComponent->BindAxis("MouseWheel", this, &ATDSCharacter::CameraZoomInput);
	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ATDSCharacter::CharacterWalk);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &ATDSCharacter::CharacterRun);
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ATDSCharacter::CharacterRun);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATDSCharacter::CharacterAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATDSCharacter::CharacterRun);
}

void ATDSCharacter::InputAxisX(const float Value)
{
	AxisX = Value;
}

void ATDSCharacter::InputAxisY(const float Value)
{
	AxisY = Value;
}

void ATDSCharacter::MovementTick()
{
	AddMovementInput(FVector(1.0f, 0.0f, 0.0f), AxisX);
	AddMovementInput(FVector(0.0f, 1.0f, 0.0f), AxisY);

	APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (MyController)
	{
		FHitResult HitResult;
		MyController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false,
		                                               HitResult);
		if (HitResult.bBlockingHit)
		{
			//rotated by cursor captured by channel
			auto rot = FRotator(
				0.0f,
				UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResult.Location).Yaw,
				0.0f);
			SetActorRotation(rot);
		}
		else
		{
			//rotated by cursor screen position
			FVector WorldLocation;
			FVector WorldDirection;
			MyController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
			FVector MousePosition = FVector(WorldLocation + WorldDirection * 500);
			auto rot = FRotator(
				0.0f,
				UKismetMathLibrary::FindLookAtRotation(GetActorLocation(),
				                                       FVector(MousePosition.X, MousePosition.Y, GetActorLocation().Z)).
				Yaw,
				0.0f);
			SetActorRotation(rot);
		}
	}
}

void ATDSCharacter::CharacterUpdate()
{
	switch (MovementState)
	{
	case EMovementState::Aim_State:
		CharacterMaxMovementSpeed = MovementInfo.AimSpeed;
		break;
	case EMovementState::Walk_State:
		CharacterMaxMovementSpeed = MovementInfo.WalkSpeed;
		break;
	case EMovementState::Run_State:
		CharacterMaxMovementSpeed = MovementInfo.RunSpeed;
		break;
	}
	GetCharacterMovement()->MaxWalkSpeed = CharacterMaxMovementSpeed;
}

void ATDSCharacter::ChangeMovementState(const EMovementState NewMovementState)
{
	MovementState = NewMovementState;
	CharacterUpdate();
}

void ATDSCharacter::CameraZoomInput(const float Value)
{
	if (Value)
	{
		CameraZoomDistance = UKismetMathLibrary::Clamp(
			Value * CameraSensitivity + GetCameraBoom()->TargetArmLength,
			CameraMinHeight,
			CameraMaxHeight);
	}
}

void ATDSCharacter::SoothingCameraZoom(const float DeltaTime) const
{
	if (CameraZoomDistance)
	{
		GetCameraBoom()->TargetArmLength = UKismetMathLibrary::FInterpTo(
			GetCameraBoom()->TargetArmLength, CameraZoomDistance, DeltaTime, CameraSmoothSpeed);
	}
}

void ATDSCharacter::CharacterAim()
{
	ChangeMovementState(EMovementState::Aim_State);
}

void ATDSCharacter::CharacterWalk()
{
	ChangeMovementState(EMovementState::Walk_State);
}

void ATDSCharacter::CharacterRun()
{
	ChangeMovementState(EMovementState::Run_State);
}
