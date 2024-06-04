// Copyright Epic Games, Inc. All Rights Reserved.

#include "TDSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
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
	CameraBoom->TargetArmLength = 1500.f;
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

	CameraZoomDistance = CameraBoom->TargetArmLength;
	CharacterMaxMovementSpeed = MovementInfo.RunSpeed;
}

void ATDSCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		if (const APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			const FVector CursorFV = TraceHitResult.ImpactNormal;
			const FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}
	MovementTick();
}

void ATDSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATDSCharacter::InputAxisX);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATDSCharacter::InputAxisY);
	PlayerInputComponent->BindAxis("MouseWheel", this, &ATDSCharacter::CameraZoomInput);
	PlayerInputComponent->BindAction("Walk", IE_Pressed, this, &ATDSCharacter::CharacterWalk);
	PlayerInputComponent->BindAction("Walk", IE_Released, this, &ATDSCharacter::CharacterRun);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATDSCharacter::CharacterSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATDSCharacter::CharacterRun);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ATDSCharacter::CharacterAim);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ATDSCharacter::CharacterRun);
}

void ATDSCharacter::InputAxisX(const float Value)
{
	AxisX = Value;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATDSCharacter::InputAxisY(const float Value)
{
	AxisY = Value;
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ATDSCharacter::MovementTick()
{
	//check if forward vector equals input vector
	FVector(1.0f * AxisX, 1.0f * AxisY, 0.0f).
		Equals(FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0.0f))
			? IsMovingInDirectionToInput = true
			: IsMovingInDirectionToInput = false;

	// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,
	//                                  FString::Printf(
	// 	                                 TEXT("IsMovingInDirectionToInput: %d"), IsMovingInDirectionToInput));


	if (IsSprinting && !IsMovingInDirectionToInput)
	{
		//if not running forward
		ChangeMovementState(ECharacterMovementState::Run_State);
	}
	else if (IsSprinting && IsMovingInDirectionToInput)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow,
		//                                  FString::Printf(TEXT("SPRINTING")));
		// if running forward
		if (!IsStaminaRecovering && !FMath::IsNearlyEqual(Stamina, 0.0f, 0.5f))
		{
			Stamina = FMath::Max(0.0f, Stamina - SpendStaminaPerTick);
			ChangeMovementState(ECharacterMovementState::Sprint_State);
			// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
			//                                  FString::Printf(TEXT("Stamina: %f"), Stamina));
			if (!StaminaRecoveryTimerHandle.IsValid() && FMath::IsNearlyEqual(Stamina, 0.0f, 0.5f))
			{
				GetWorldTimerManager().SetTimer(StaminaRecoveryTimerHandle, this, &ATDSCharacter::RecoveryStamina,
				                                GetWorld()->GetDeltaSeconds(), true, 3.0f);
			}
		}
		else
		{
			ChangeMovementState(ECharacterMovementState::Run_State);
		}
	}

	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
	//                                  FString::Printf(TEXT("speed: %f"), CharacterMaxMovementSpeed));
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
	//                                  FString::Printf(TEXT("speed: %hhd"), MovementState));

	const APlayerController* MyController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (MyController)
	{
		FHitResult HitResult;
		MyController->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1), false,
		                                               HitResult);
		if (HitResult.bBlockingHit)
		{
			//rotated by cursor captured by channel
			SetActorRotation(FRotator(
				0.0f,
				UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), HitResult.Location).Yaw,
				0.0f));
		}
		else
		{
			//rotated by cursor screen position
			FVector WorldLocation;
			FVector WorldDirection;
			MyController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
			const FVector MousePosition = FVector(WorldLocation + WorldDirection * 500);
			SetActorRotation(FRotator(
				0.0f,
				UKismetMathLibrary::FindLookAtRotation(
					GetActorLocation(), FVector(MousePosition.X, MousePosition.Y, GetActorLocation().Z)).Yaw,
				0.0f));
		}
	}
}

void ATDSCharacter::CharacterUpdate()
{
	switch (MovementState)
	{
	case ECharacterMovementState::Run_State:
		CharacterMaxMovementSpeed = MovementInfo.RunSpeed;
		break;
	case ECharacterMovementState::Aim_State:
		CharacterMaxMovementSpeed = MovementInfo.AimSpeed;
		break;
	case ECharacterMovementState::Walk_State:
		CharacterMaxMovementSpeed = MovementInfo.WalkSpeed;
		break;
	case ECharacterMovementState::Sprint_State:
		CharacterMaxMovementSpeed = MovementInfo.SprintSpeed;
		break;
	}
	GetCharacterMovement()->MaxWalkSpeed = CharacterMaxMovementSpeed;
}

void ATDSCharacter::ChangeMovementState(const ECharacterMovementState NewMovementState)
{
	MovementState = NewMovementState;
	CharacterUpdate();
}

void ATDSCharacter::CameraZoomInput(const float Value)
{
	if (Value)
	{
		CameraZoomDistance = FMath::Min<float>(
			FMath::Max<float>(CameraBoom->TargetArmLength + CameraSensitivity * Value, CameraMinHeight),
			CameraMaxHeight);
		if (!CameraSmoothTimerHandle.IsValid() && !FMath::IsNearlyEqual(CameraBoom->TargetArmLength, CameraZoomDistance,
		                                                                0.5f))
		{
			GetWorldTimerManager().SetTimer(CameraSmoothTimerHandle, this, &ATDSCharacter::SoothingCameraZoom,
			                                GetWorld()->GetDeltaSeconds(), true, 0.0f);
		}
	}
}

void ATDSCharacter::SoothingCameraZoom()
{
	if (!FMath::IsNearlyEqual(CameraBoom->TargetArmLength, CameraZoomDistance, 0.5f))
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, CameraZoomDistance,
		                                               GetWorld()->GetDeltaSeconds(), CameraZoomSmoothness);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CameraSmoothTimerHandle);
	}
}

void ATDSCharacter::CharacterAim()
{
	IsAiming = true;
	IsWalking = IsRunning = IsSprinting = false;
	ChangeMovementState(ECharacterMovementState::Aim_State);
	//PrintState();
}

void ATDSCharacter::CharacterWalk()
{
	IsWalking = true;
	IsAiming = IsRunning = IsSprinting = false;
	ChangeMovementState(ECharacterMovementState::Walk_State);
	//PrintState();
}

void ATDSCharacter::CharacterRun()
{
	IsRunning = true;
	IsAiming = IsWalking = IsSprinting = false;
	ChangeMovementState(ECharacterMovementState::Run_State);
	//PrintState();
}

void ATDSCharacter::CharacterSprint()
{
	IsSprinting = true;
	IsAiming = IsWalking = IsRunning = false;
	ChangeMovementState(ECharacterMovementState::Sprint_State);
	//PrintState();
}

void ATDSCharacter::PrintState() const
{
	UE_LOG(LogTemp, Log, TEXT("IsAiming: %d, IsWalking: %d, IsRunning: %d, IsSprinting: %d"),
	       IsAiming, IsWalking, IsRunning, IsSprinting)
}

void ATDSCharacter::RecoveryStamina()
{
	ChangeMovementState(ECharacterMovementState::Run_State);
	Stamina = FMath::Min(MaxStamina, Stamina + RecoveryStaminaPerTick);
	IsStaminaRecovering = true;
	// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
	//                                  FString::Printf(TEXT("Stamina recovering %f"), Stamina));
	if (FMath::IsNearlyEqual(Stamina, MaxStamina))
	{
		Stamina = MaxStamina;
		IsStaminaRecovering = false;
		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
		//                                  FString::Printf(TEXT("Stamina recovery finished")));
		GetWorldTimerManager().ClearTimer(StaminaRecoveryTimerHandle);
	}
}
