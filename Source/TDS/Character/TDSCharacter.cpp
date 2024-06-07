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
#include "TDS/Game/TDSGameInstance.h"
#include "TDS/Items/Weapon/WeaponDefault.h"

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

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	CameraZoomDistance = CameraBoom->TargetArmLength;
	CharacterMaxMovementSpeed = MovementInfo.RunSpeed;
}

void ATDSCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitWeapon(InitWeaponName);

	if (CursorMaterial)
	{
		CurrentCursor = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), CursorMaterial, CursorSize, FVector(0));
	}
}

void ATDSCharacter::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentCursor)
	{
		if (const APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			const FVector CursorFV = TraceHitResult.ImpactNormal;
			const FRotator CursorR = CursorFV.Rotation();
			CurrentCursor->SetWorldLocation(TraceHitResult.Location);
			CurrentCursor->SetWorldRotation(CursorR);
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
	PlayerInputComponent->BindAction("FireEvent", IE_Pressed, this, &ATDSCharacter::InputAttackPressed);
	PlayerInputComponent->BindAction("FireEvent", IE_Released, this, &ATDSCharacter::InputAttackReleased);
	PlayerInputComponent->BindAction("ReloadEvent", IE_Released, this, &ATDSCharacter::TryReloadWeapon);
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

void ATDSCharacter::InputAttackPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("InputAttackPressed"));
	AttackCharEvent(true);
}

void ATDSCharacter::InputAttackReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("InputAttackReleased"));
	AttackCharEvent(false);
}

void ATDSCharacter::MovementTick()
{
	//check if forward vector equals input vector
	FVector(1.0f * AxisX, 1.0f * AxisY, 0.0f).
		Equals(FVector(GetActorForwardVector().X, GetActorForwardVector().Y, 0.0f))
			? IsMovingInDirectionToInput = true
			: IsMovingInDirectionToInput = false;

	if (IsSprinting && !IsMovingInDirectionToInput)
	{
		//if not running forward
		ChangeMovementState(ECharacterMovementState::Run_State);
	}
	else if (IsSprinting && IsMovingInDirectionToInput)
	{
		// if running forward
		if (!IsStaminaRecovering && !FMath::IsNearlyEqual(Stamina, 0.0f, 0.5f))
		{
			Stamina = FMath::Max(0.0f, Stamina - SpendStaminaPerTick);
			ChangeMovementState(ECharacterMovementState::Sprint_State);
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
	case ECharacterMovementState::AimWalk_State:
		CharacterMaxMovementSpeed = MovementInfo.AimWalkSpeed;
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

void ATDSCharacter::AttackCharEvent(const bool bIsFiring)
{
	if (GetCurrentWeapon())
	{
		GetCurrentWeapon()->SetWeaponStateFire(bIsFiring);
	}
	else UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
}

AWeaponDefault* ATDSCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

void ATDSCharacter::InitWeapon(const FName IdWeaponName)
{
	const UTDSGameInstance* GI = Cast<UTDSGameInstance>(GetGameInstance());
	if (GI)
	{
		FWeaponInfo WeaponInfo;
		if (GI->GetWeaponInfoByName(IdWeaponName, WeaponInfo))
		{
			if (WeaponInfo.WeaponClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("WeaponInfo InitWeapon"));
				const FVector SpawnLocation = FVector::ZeroVector;
				const FRotator SpawnRotation = FRotator::ZeroRotator;
	
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();
	
				AWeaponDefault* Weapon = Cast<AWeaponDefault>(GetWorld()->SpawnActor(WeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (Weapon)
				{
					const FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					Weapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = Weapon;
					
					Weapon->WeaponSetting = WeaponInfo;
					Weapon->WeaponInfo.Round = WeaponInfo.MaxRound;
					//Remove !!! Debug
					Weapon->ReloadTime = WeaponInfo.ReloadTime;
					Weapon->UpdateStateWeapon(MovementState);
	
					Weapon->OnWeaponReloadStart.AddDynamic(this, &ATDSCharacter::WeaponReloadStart);
					Weapon->OnWeaponReloadEnd.AddDynamic(this, &ATDSCharacter::WeaponReloadEnd);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::InitWeapon - Weapon not found in table -NULL"));
		}
	}
}

UDecalComponent* ATDSCharacter::GetCursorToWorld() const
{
	return CurrentCursor;
}


void ATDSCharacter::TryReloadWeapon()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetWeaponRound() <= CurrentWeapon->WeaponSetting.MaxRound)
			CurrentWeapon->InitReload();
	}
}

void ATDSCharacter::WeaponReloadStart(UAnimMontage* Anim)
{
	WeaponReloadStart_BP(Anim);
}

void ATDSCharacter::WeaponReloadEnd()
{
	WeaponReloadEnd_BP();
}

void ATDSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

void ATDSCharacter::WeaponReloadEnd_BP_Implementation()
{
	// in BP
}