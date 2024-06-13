#include "TDSCharacter.h"
#include "TDSInventoryComponent.h"
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
	CurrentWeapon->DebugShowStatus();
}

void ATDSCharacter::InputAttackReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("InputAttackReleased"));
	AttackCharEvent(false);
}

void ATDSCharacter::SetWeaponDisplacement(const FVector_NetQuantize Location) const
{
	if (CurrentWeapon)
	{
		FVector Displacement = FVector(0);
		switch (MovementState)
		{
		case ECharacterMovementState::Aim_State:
			Displacement = FVector(0.0f, 0.0f, 160.0f);
			CurrentWeapon->ShouldReduceDispersion = true;
			break;
		case ECharacterMovementState::AimWalk_State:
			CurrentWeapon->ShouldReduceDispersion = true;
			Displacement = FVector(0.0f, 0.0f, 160.0f);
			break;
		case ECharacterMovementState::Walk_State:
			Displacement = FVector(0.0f, 0.0f, 120.0f);
			CurrentWeapon->ShouldReduceDispersion = false;
			break;
		case ECharacterMovementState::Run_State:
			Displacement = FVector(0.0f, 0.0f, 120.0f);
			CurrentWeapon->ShouldReduceDispersion = false;
			break;
		case ECharacterMovementState::Sprint_State:
			break;
		default:
			break;
		}
		CurrentWeapon->ShootEndLocation = Location + Displacement;
		//aim cursor like 3d Widget?
	}
}

void ATDSCharacter::SetWeaponDebugState(const bool State) const
{
	CurrentWeapon->SetDebugState(State);
}

bool ATDSCharacter::GetWeaponDebugState() const
{
	return CurrentWeapon->GetDebugState();
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

			SetWeaponDisplacement(HitResult.Location);
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
			SetWeaponDisplacement(FVector(MousePosition.X, MousePosition.Y, GetActorLocation().Z));
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

void ATDSCharacter::SetInitWeaponName(FString WeaponName)
{
	InitWeaponName = FName(WeaponName);
	UE_LOG(LogTemp, Warning, TEXT("In BP chosen weapon is %s"), *WeaponName)
	CurrentWeapon->Destroy();
	InitWeapon(InitWeaponName);
	UE_LOG(LogTemp, Log, TEXT("Seted weapon is %s"), *CurrentWeapon->WeaponSetting.WeaponClass->GetFName().ToString())
}

void ATDSCharacter::AttackCharEvent(const bool bIsFiring)
{
	if (GetCurrentWeapon())
	{
		GetCurrentWeapon()->SetWeaponStateFire(bIsFiring);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ATPSCharacter::AttackCharEvent - CurrentWeapon -NULL"));
	}
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
				const FVector SpawnLocation = FVector::ZeroVector;
				const FRotator SpawnRotation = FRotator::ZeroRotator;

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();

				AWeaponDefault* Weapon = Cast<AWeaponDefault>(
					GetWorld()->SpawnActor(WeaponInfo.WeaponClass, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (Weapon)
				{
					const FAttachmentTransformRules Rule(EAttachmentRule::SnapToTarget, false);
					Weapon->AttachToComponent(GetMesh(), Rule, FName("WeaponSocketRightHand"));
					CurrentWeapon = Weapon;
					Weapon->WeaponSetting = WeaponInfo;
					Weapon->WeaponInfo.Round = WeaponInfo.MaxRound;
					Weapon->ReloadTime = WeaponInfo.ReloadTime;
					Weapon->UpdateStateWeapon(MovementState);
					(MovementState == ECharacterMovementState::Aim_State || MovementState ==
						ECharacterMovementState::AimWalk_State)
						? Weapon->UpdateWeaponAimingState(true)
						: Weapon->UpdateWeaponAimingState(false);
					Weapon->OnWeaponReloadStart.AddDynamic(this, &ATDSCharacter::WeaponReloadStart);
					Weapon->OnWeaponReloadEnd.AddDynamic(this, &ATDSCharacter::WeaponReloadEnd);
					Weapon->OnWeaponFireStart.AddDynamic(this, &ATDSCharacter::WeaponFireStart);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning,
			       TEXT("ATPSCharacter::InitWeapon - Weapon not found in table, set weapon or check spelling"));
		}
	}
}

void ATDSCharacter::WeaponFireStart(UAnimMontage* Anim)
{
	if (CurrentWeapon)
		WeaponFireStart_BP(Anim);
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

void ATDSCharacter::WeaponReloadEnd(const bool bIsSuccess, const int32 AmmoTake)
{
	if (InventoryComponent && CurrentWeapon)
	{
		InventoryComponent->AmmoSlotChangeValue(CurrentWeapon->WeaponSetting.WeaponType, AmmoTake);
		InventoryComponent->SetAdditionalInfoWeapon(CurrentIndexWeapon, CurrentWeapon->AdditionalWeaponInfo);
	}
	WeaponReloadEnd_BP(bIsSuccess);
}

void ATDSCharacter::WeaponReloadStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

void ATDSCharacter::WeaponReloadEnd_BP_Implementation(bool bIsSuccess)
{
	// in BP
}

void ATDSCharacter::WeaponFireStart_BP_Implementation(UAnimMontage* Anim)
{
	// in BP
}

void ATDSCharacter::TrySwitchNextWeapon() const
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		const int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			if (InventoryComponent->SwitchWeaponToIndex(CurrentIndexWeapon + 1, OldIndex, OldInfo, true))
			{
			}
		}
	}
}

void ATDSCharacter::TrySwitchPreviousWeapon() const
{
	if (InventoryComponent->WeaponSlots.Num() > 1)
	{
		const int8 OldIndex = CurrentIndexWeapon;
		FAdditionalWeaponInfo OldInfo;
		if (CurrentWeapon)
		{
			OldInfo = CurrentWeapon->AdditionalWeaponInfo;
			if (CurrentWeapon->WeaponReloading)
				CurrentWeapon->CancelReload();
		}

		if (InventoryComponent)
		{
			//InventoryComponent->SetAdditionalInfoWeapon(OldIndex, GetCurrentWeapon()->AdditionalWeaponInfo);
			if (InventoryComponent->SwitchWeaponToIndex(CurrentIndexWeapon - 1, OldIndex, OldInfo, false))
			{
			}
		}
	}
}
