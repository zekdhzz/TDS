#include "WeaponDefault.h"

#include "DrawDebugHelpers.h"
#include "ProjectileDefault.h"
#include "Components/ArrowComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TDS/Character/TDSInventoryComponent.h"

AWeaponDefault::AWeaponDefault()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	RootComponent = SceneComponent;

	SkeletalMeshWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	SkeletalMeshWeapon->SetGenerateOverlapEvents(false);
	SkeletalMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	SkeletalMeshWeapon->SetupAttachment(RootComponent);

	StaticMeshWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh "));
	StaticMeshWeapon->SetGenerateOverlapEvents(false);
	StaticMeshWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMeshWeapon->SetupAttachment(RootComponent);

	ShootLocation = CreateDefaultSubobject<UArrowComponent>(TEXT("ShootLocation"));
	ShootLocation->SetupAttachment(RootComponent);
}


void AWeaponDefault::BeginPlay()
{
	Super::BeginPlay();

	WeaponInit();
}

void AWeaponDefault::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	FireTick(DeltaTime);
	ReloadTick(DeltaTime);
	DispersionTick(DeltaTime);
	ShellDropTick(DeltaTime);
	ClipDropTick(DeltaTime);
}


void AWeaponDefault::FireTick(const float DeltaTime)
{
	if (GetWeaponRound() > 0)
	{
		if (WeaponFiring)
		{
			if (FireTimer < 0.f)
			{
				if (!WeaponReloading)
				{
					Fire();
				}
			}
			else
			{
				FireTimer -= DeltaTime;
			}
		}
	}
	else
	{
		if (!WeaponReloading)
		{
			InitReload();
		}
	}
}

void AWeaponDefault::ReloadTick(const float DeltaTime)
{
	if (WeaponReloading)
	{
		if (ReloadTimer < 0.0f)
		{
			FinishReload();
		}
		else
		{
			ReloadTimer -= DeltaTime;
		}
	}
}

void AWeaponDefault::DispersionTick(float DeltaTime)
{
	if (!WeaponReloading)
	{
		if (!WeaponFiring)
		{
			if (ShouldReduceDispersion)
				CurrentDispersion = CurrentDispersion - CurrentDispersionReduction;
			else
				CurrentDispersion = CurrentDispersion + CurrentDispersionReduction;
		}

		if (CurrentDispersion < CurrentDispersionMin)
		{
			CurrentDispersion = CurrentDispersionMin;
		}
		else
		{
			if (CurrentDispersion > CurrentDispersionMax)
			{
				CurrentDispersion = CurrentDispersionMax;
			}
		}
	}
	// if (ShowDebug)
	// 	UE_LOG(LogTemp, Warning, TEXT("Dispersion: MAX = %f. MIN = %f. Current = %f"), CurrentDispersionMax,
	//        CurrentDispersionMin, CurrentDispersion);
}

void AWeaponDefault::WeaponInit()
{
	if (SkeletalMeshWeapon && !SkeletalMeshWeapon->SkeletalMesh)
	{
		SkeletalMeshWeapon->DestroyComponent(true);
	}

	if (StaticMeshWeapon && !StaticMeshWeapon->GetStaticMesh())
	{
		StaticMeshWeapon->DestroyComponent();
	}
	UpdateStateWeapon(ECharacterMovementState::Run_State);
}

void AWeaponDefault::UpdateWeaponAimingState(const bool bIsAiming)
{
	WeaponAiming = bIsAiming;
}

void AWeaponDefault::SetWeaponStateFire(const bool bIsFire)
{
	if (CheckWeaponCanFire())
	{
		WeaponFiring = bIsFire;
	}
	else
	{
		WeaponFiring = false;
		FireTimer = 0.005f;
	}
}

bool AWeaponDefault::CheckWeaponCanFire() const
{
	return !BlockFire;
}

FProjectileInfo AWeaponDefault::GetProjectile()
{
	return WeaponSetting.ProjectileSetting;
}

void AWeaponDefault::Fire()
{
	UAnimMontage* AnimToPlay;

	if (WeaponAiming)
		AnimToPlay = WeaponSetting.AnimWeaponInfo.AnimCharFireAim;
	else
		AnimToPlay = WeaponSetting.AnimWeaponInfo.AnimCharFire;

	if (WeaponSetting.AnimWeaponInfo.AnimWeaponFire)
	{
		AnimWeaponStart(WeaponSetting.AnimWeaponInfo.AnimWeaponFire);
	}

	if (WeaponSetting.ShellBullets.DropMesh)
	{
		if (WeaponSetting.ShellBullets.DropMeshTime < 0.0f)
		{
			InitDropMesh(WeaponSetting.ShellBullets.DropMesh, WeaponSetting.ShellBullets.DropMeshOffset,
			             WeaponSetting.ShellBullets.DropMeshImpulseDir, WeaponSetting.ShellBullets.DropMeshLifeTime,
			             WeaponSetting.ShellBullets.ImpulseRandomDispersion, WeaponSetting.ShellBullets.PowerImpulse,
			             WeaponSetting.ShellBullets.CustomMass);
		}
		else
		{
			DropShellFlag = true;
			DropShellTimer = WeaponSetting.ShellBullets.DropMeshTime;
		}
	}

	//UE_LOG(LogTemp, Warning, TEXT("AWeaponDefault Fire"));
	FireTimer = WeaponSetting.RateOfFire;
	WeaponInfo.Round = WeaponInfo.Round - 1;
	UE_LOG(LogTemp, Warning, TEXT("Rounds in clip %i"), WeaponInfo.Round);

	OnWeaponFireStart.Broadcast(AnimToPlay);

	ChangeDispersionByShot();

	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon,
	                                       ShootLocation->GetComponentLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon,
	                                         ShootLocation->GetComponentTransform());

	const int8 NumberProjectile = GetNumberProjectileByShot();

	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();
		FVector EndTraceLocation;
		for (int8 i = 0; i < NumberProjectile; i++) //Shotgun
		{
			EndTraceLocation = GetFireEndLocation();
			//FVector Dir = EndTraceLocation - SpawnLocation;
			//Dir.Normalize();
			//FRotator SpawnTraceRotation = FMatrix(Dir, FVector(0, 1, 0), FVector(0, 0, 1), FVector::ZeroVector).Rotator();
			if (ProjectileInfo.Projectile)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetInstigator();
				SpawnParams.bDeferConstruction = true;
				AProjectileDefault* Projectile = Cast<AProjectileDefault>(
					GetWorld()->SpawnActor(ProjectileInfo.Projectile, &SpawnLocation, &SpawnRotation, SpawnParams));
				if (Projectile)
				{
					Projectile->InitProjectile(WeaponSetting.ProjectileSetting, ShowDebug);
					UGameplayStatics::FinishSpawningActor(
						Projectile, FTransform(SpawnRotation, SpawnLocation, FVector(1.0f, 1.0f, 1.0f)));
				}
				UE_LOG(LogTemp, Warning, TEXT("Projectile"));
			}
			else
			{
				FHitResult Hit;
				TArray<AActor*> Actors;

				EDrawDebugTrace::Type DebugTrace;
				if (ShowDebug)
				{
					DrawDebugLine(GetWorld(), SpawnLocation,
					              SpawnLocation + ShootLocation->GetForwardVector() * WeaponSetting.DistacneTrace,
					              FColor::Black, false, 5.f, (uint8)'\000', 0.5f);
					DebugTrace = EDrawDebugTrace::ForDuration;
				}
				else
					DebugTrace = EDrawDebugTrace::None;

				UKismetSystemLibrary::LineTraceSingle(GetWorld(), SpawnLocation,
				                                      EndTraceLocation * WeaponSetting.DistacneTrace,
				                                      ETraceTypeQuery::TraceTypeQuery4, false, Actors, DebugTrace, Hit,
				                                      true, FLinearColor::Red, FLinearColor::Green, 5.0f);
				if (Hit.GetActor() && Hit.PhysMaterial.IsValid())
				{
					EPhysicalSurface Surface = UGameplayStatics::GetSurfaceType(Hit);

					if (WeaponSetting.ProjectileSetting.HitDecals.Contains(Surface))
					{
						UMaterialInterface* Material = WeaponSetting.ProjectileSetting.HitDecals[Surface];

						if (Material && Hit.GetComponent())
						{
							SpawnTraceHitDecal(Material, Hit);
						}
					}
					if (WeaponSetting.ProjectileSetting.HitFXs.Contains(Surface))
					{
						UParticleSystem* Particle = WeaponSetting.ProjectileSetting.HitFXs[Surface];
						if (Particle)
						{
							SpawnTraceHitFX(Particle, Hit);
						}
					}
					if (WeaponSetting.ProjectileSetting.HitSound.Contains(Surface))
					{
						SpawnTraceHitSound(WeaponSetting.ProjectileSetting.HitSound[Surface], Hit);
					}
					UGameplayStatics::ApplyPointDamage(Hit.GetActor(), WeaponSetting.ProjectileSetting.ProjectileDamage,
					                                   Hit.TraceStart, Hit, GetInstigatorController(), this, nullptr);
				}
				UE_LOG(LogTemp, Warning, TEXT("Hitscan"));
			}
		}
	}
}

void AWeaponDefault::UpdateStateWeapon(const ECharacterMovementState MovementState)
{
	BlockFire = false;
	switch (MovementState)
	{
	case ECharacterMovementState::Aim_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Aim_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case ECharacterMovementState::AimWalk_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.AimWalk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case ECharacterMovementState::Walk_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Walk_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case ECharacterMovementState::Run_State:
		CurrentDispersionMax = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMax;
		CurrentDispersionMin = WeaponSetting.DispersionWeapon.Run_StateDispersionAimMin;
		CurrentDispersionRecoil = WeaponSetting.DispersionWeapon.Run_StateDispersionAimRecoil;
		CurrentDispersionReduction = WeaponSetting.DispersionWeapon.Aim_StateDispersionReduction;
		break;
	case ECharacterMovementState::Sprint_State:
		//Block Fire
		BlockFire = true;
		SetWeaponStateFire(false); //set fire trigger to false
		break;
	default:
		break;
	}
}

void AWeaponDefault::ChangeDispersionByShot()
{
	CurrentDispersion += CurrentDispersionRecoil;
}

float AWeaponDefault::GetCurrentDispersion() const
{
	return CurrentDispersion;
}

FVector AWeaponDefault::ApplyDispersionToShoot(const FVector DirectionShoot) const
{
	return FMath::VRandCone(DirectionShoot, GetCurrentDispersion() * PI / 180.f);
}

FVector AWeaponDefault::GetFireEndLocation() const
{
	FVector EndLocation;
	const FVector TmpV = (ShootLocation->GetComponentLocation() - ShootEndLocation);
	//UE_LOG(LogTemp, Warning, TEXT("Vector: X = %f. Y = %f. Size = %f"), tmpV.X, tmpV.Y, tmpV.Size());

	if (TmpV.Size() > SizeVectorToChangeShootDirectionLogic)
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(
			(ShootLocation->GetComponentLocation() - ShootEndLocation).GetSafeNormal()) * -20000.0f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(),
			              -(ShootLocation->GetComponentLocation() - ShootEndLocation), WeaponSetting.DistacneTrace,
			              GetCurrentDispersion() * PI / 180.f, GetCurrentDispersion() * PI / 180.f, 32, FColor::Emerald,
			              false, .1f, (uint8)'\000', 1.0f);
	}
	else
	{
		EndLocation = ShootLocation->GetComponentLocation() + ApplyDispersionToShoot(ShootLocation->GetForwardVector())
			* 20000.0f;
		if (ShowDebug)
			DrawDebugCone(GetWorld(), ShootLocation->GetComponentLocation(), ShootLocation->GetForwardVector(),
			              WeaponSetting.DistacneTrace, GetCurrentDispersion() * PI / 180.f,
			              GetCurrentDispersion() * PI / 180.f, 32, FColor::Emerald, false, .1f, (uint8)'\000', 1.0f);
	}
	if (ShowDebug)
	{
		//direction weapon look
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(),
		              ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector() * 500.0f, FColor::Cyan,
		              false, 5.f, (uint8)'\000', 0.5f);
		//direction projectile must fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), ShootEndLocation, FColor::Red, false, 5.f,
		              (uint8)'\000', 0.5f);
		//Direction Projectile Current fly
		DrawDebugLine(GetWorld(), ShootLocation->GetComponentLocation(), EndLocation, FColor::Black, false, 5.f,
		              (uint8)'\000', 0.5f);

		//DrawDebugSphere(GetWorld(), ShootLocation->GetComponentLocation() + ShootLocation->GetForwardVector()*SizeVectorToChangeShootDirectionLogic, 10.f, 8, FColor::Red, false, 4.0f);
	}


	return EndLocation;
}

int8 AWeaponDefault::GetNumberProjectileByShot() const
{
	return WeaponSetting.NumberProjectileByShot;
}

int32 AWeaponDefault::GetWeaponRound() const
{
	return WeaponInfo.Round;
}

int8 AWeaponDefault::GetAvailableAmmoForReload() const
{
	int8 AmmoForWeapon = WeaponSetting.MaxRound;
	if (GetOwner())
	{
		UTDSInventoryComponent* MyInv = Cast<UTDSInventoryComponent>(
			GetOwner()->GetComponentByClass(UTDSInventoryComponent::StaticClass()));
		if (MyInv)
		{
			if (MyInv->CheckAmmoForWeapon(WeaponSetting.WeaponType, AmmoForWeapon))
			{
				AmmoForWeapon = AmmoForWeapon; ///?????
			}
		}
	}
	return AmmoForWeapon;
}

void AWeaponDefault::InitReload()
{
	WeaponReloading = true;
	ReloadTimer = WeaponSetting.ReloadTime;

	UAnimMontage* AnimCharReload;
	if (WeaponAiming)
		AnimCharReload = WeaponSetting.AnimWeaponInfo.AnimCharReloadAim;
	else
		AnimCharReload = WeaponSetting.AnimWeaponInfo.AnimCharReload;
	OnWeaponReloadStart.Broadcast(AnimCharReload);

	UAnimMontage* AnimWeaponReload;
	if (WeaponAiming)
		AnimWeaponReload = WeaponSetting.AnimWeaponInfo.AnimWeaponReloadAim;
	else
		AnimWeaponReload = WeaponSetting.AnimWeaponInfo.AnimWeaponReload;

	if (WeaponSetting.AnimWeaponInfo.AnimWeaponReload
		&& SkeletalMeshWeapon
		&& SkeletalMeshWeapon->GetAnimInstance())
	{
		AnimWeaponStart(AnimWeaponReload);
	}

	if (WeaponSetting.ClipDropMesh.DropMesh)
	{
		DropClipFlag = true;
		DropClipTimer = WeaponSetting.ClipDropMesh.DropMeshTime;
	}
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;

	const int8 AvailableAmmoFromInventory = GetAvailableAmmoForReload();
	int8 AmmoNeedTakeFromInv;
	const int8 NeedToReload = WeaponSetting.MaxRound - AdditionalWeaponInfo.Round;
	if (NeedToReload > AvailableAmmoFromInventory)
	{
		AdditionalWeaponInfo.Round = AvailableAmmoFromInventory;
		AmmoNeedTakeFromInv = AvailableAmmoFromInventory;
	}
	else
	{
		AdditionalWeaponInfo.Round += NeedToReload;
		AmmoNeedTakeFromInv = NeedToReload;
	}
	OnWeaponReloadEnd.Broadcast(true, -AmmoNeedTakeFromInv);
}

void AWeaponDefault::CancelReload()
{
	WeaponReloading = false;
	if (SkeletalMeshWeapon && SkeletalMeshWeapon->GetAnimInstance())
	{
		SkeletalMeshWeapon->GetAnimInstance()->StopAllMontages(0.15f);
	}
	OnWeaponReloadEnd.Broadcast(false, 0);
	DropClipFlag = false;
}

bool AWeaponDefault::CheckCanWeaponReload() const
{
	bool bResult = true;
	if (GetOwner())
	{
		UTDSInventoryComponent* MyInv = Cast<UTDSInventoryComponent>(
			GetOwner()->GetComponentByClass(UTDSInventoryComponent::StaticClass()));
		if (MyInv)
		{
			int8 AvailableAmmoForWeapon;
			if (!MyInv->CheckAmmoForWeapon(WeaponSetting.WeaponType, AvailableAmmoForWeapon))
			{
				bResult = false;
			}
		}
	}

	return bResult;
}

void AWeaponDefault::AnimWeaponStart(UAnimMontage* WeaponAnim) const
{
	if (WeaponAnim
		&& SkeletalMeshWeapon
		&& SkeletalMeshWeapon->GetAnimInstance())
	{
		SkeletalMeshWeapon->GetAnimInstance()->Montage_Play(WeaponAnim);
	}
}

void AWeaponDefault::InitDropMesh(UStaticMesh* DropMesh, const FTransform& Offset, const FVector DropImpulseDirection,
                                  const float LifeTimeMesh, const float ImpulseRandomDispersion,
                                  const float PowerImpulse,
                                  const float CustomMass)
{
	if (DropMesh)
	{
		FTransform Transform;
		const FVector LocalDir = this->GetActorForwardVector() * Offset.GetLocation().X + this->GetActorRightVector() *
			Offset.GetLocation().Y + this->GetActorUpVector() * Offset.GetLocation().Z;
		Transform.SetLocation(GetActorLocation() + LocalDir);
		Transform.SetScale3D(Offset.GetScale3D());
		Transform.SetRotation((GetActorRotation() + Offset.Rotator()).Quaternion());

		ShellDropFire(DropMesh, Transform, DropImpulseDirection, LifeTimeMesh, ImpulseRandomDispersion, PowerImpulse,
		              CustomMass, LocalDir);
	}
}

void AWeaponDefault::ShellDropTick(const float DeltaTime)
{
	if (DropShellFlag)
	{
		if (DropShellTimer < 0.0f)
		{
			DropShellFlag = false;
			InitDropMesh(WeaponSetting.ShellBullets.DropMesh, WeaponSetting.ShellBullets.DropMeshOffset,
			             WeaponSetting.ShellBullets.DropMeshImpulseDir, WeaponSetting.ShellBullets.DropMeshLifeTime,
			             WeaponSetting.ShellBullets.ImpulseRandomDispersion, WeaponSetting.ShellBullets.PowerImpulse,
			             WeaponSetting.ShellBullets.CustomMass);
		}
		else
			DropShellTimer -= DeltaTime;
	}
}

void AWeaponDefault::ShellDropFire(UStaticMesh* DropMesh, const FTransform& Offset,
                                   const FVector DropImpulseDirection, const float LifeTimeMesh,
                                   const float ImpulseRandomDispersion,
                                   const float PowerImpulse,
                                   const float CustomMass, FVector LocalDir)
{
	FActorSpawnParameters Param;
	Param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Param.Owner = this;


	AStaticMeshActor* NewActor = GetWorld()->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), Offset, Param);
	if (NewActor && NewActor->GetStaticMeshComponent())
	{
		NewActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("IgnoreOnlyPawn"));
		NewActor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

		//set parameter for new actor
		NewActor->SetActorTickEnabled(false);
		//NewActor->InitialLifeSpan = LifeTimeMesh;
		NewActor->SetLifeSpan(LifeTimeMesh);

		NewActor->GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;
		NewActor->GetStaticMeshComponent()->SetSimulatePhysics(true);
		NewActor->GetStaticMeshComponent()->SetStaticMesh(DropMesh);

		NewActor->GetStaticMeshComponent()->SetCollisionResponseToChannel(
			ECC_GameTraceChannel1, ECollisionResponse::ECR_Ignore);
		NewActor->GetStaticMeshComponent()->SetCollisionResponseToChannel(
			ECC_GameTraceChannel2, ECollisionResponse::ECR_Ignore);
		NewActor->GetStaticMeshComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
		NewActor->GetStaticMeshComponent()->SetCollisionResponseToChannel(
			ECC_WorldStatic, ECollisionResponse::ECR_Block);
		NewActor->GetStaticMeshComponent()->SetCollisionResponseToChannel(
			ECC_WorldDynamic, ECollisionResponse::ECR_Block);
		NewActor->GetStaticMeshComponent()->SetCollisionResponseToChannel(
			ECC_PhysicsBody, ECollisionResponse::ECR_Block);

		if (CustomMass > 0.0f)
		{
			NewActor->GetStaticMeshComponent()->SetMassOverrideInKg(NAME_None, CustomMass, true);
		}

		if (!DropImpulseDirection.IsNearlyZero())
		{
			FVector FinalDir;
			LocalDir = LocalDir + (DropImpulseDirection * 1000.0f);

			if (!FMath::IsNearlyZero(ImpulseRandomDispersion))
				FinalDir += UKismetMathLibrary::RandomUnitVectorInConeInDegrees(LocalDir, ImpulseRandomDispersion);
			//FinalDir.GetSafeNormal(0.0001f);

			NewActor->GetStaticMeshComponent()->AddImpulse(FinalDir * PowerImpulse);
		}
	}
}

void AWeaponDefault::ClipDropTick(const float DeltaTime)
{
	if (DropClipFlag)
	{
		if (DropClipTimer < 0.0f)
		{
			DropClipFlag = false;
			InitDropMesh(WeaponSetting.ClipDropMesh.DropMesh, WeaponSetting.ClipDropMesh.DropMeshOffset,
			             WeaponSetting.ClipDropMesh.DropMeshImpulseDir, WeaponSetting.ClipDropMesh.DropMeshLifeTime,
			             WeaponSetting.ClipDropMesh.ImpulseRandomDispersion, WeaponSetting.ClipDropMesh.PowerImpulse,
			             WeaponSetting.ClipDropMesh.CustomMass);
		}
		else
			DropClipTimer -= DeltaTime;
	}
}

bool AWeaponDefault::GetDebugState() const
{
	return ShowDebug;
}

void AWeaponDefault::SetDebugState(const bool IsDebugMode)
{
	ShowDebug = IsDebugMode;
}

void AWeaponDefault::SpawnTraceHitDecal(UMaterialInterface* DecalMaterial, const FHitResult& HitResult)
{
	UGameplayStatics::SpawnDecalAttached(DecalMaterial, FVector(20.0f), HitResult.GetComponent(), NAME_None,
	                                     HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation(),
	                                     EAttachLocation::KeepWorldPosition, 10.0f);
}

void AWeaponDefault::SpawnTraceHitFX(UParticleSystem* FxTemplate, const FHitResult& HitResult) const
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FxTemplate,
	                                         FTransform(HitResult.ImpactNormal.Rotation(), HitResult.ImpactPoint,
	                                                    FVector(1.0f)));
}

void AWeaponDefault::SpawnTraceHitSound(USoundBase* HitSound, const FHitResult& HitResult) const
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, HitResult.ImpactPoint);
}

void AWeaponDefault::DebugShowStatus() const
{
	UE_LOG(LogTemp, Warning, TEXT("WeaponAiming - %i WeaponFiring - %i WeaponReloading - %i BlockFire - %i"),
	       WeaponAiming, WeaponFiring, WeaponReloading, BlockFire);
}
