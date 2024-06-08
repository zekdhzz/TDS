#include "WeaponDefault.h"

#include "DrawDebugHelpers.h"
#include "ProjectileDefault.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"

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
}


void AWeaponDefault::FireTick(float DeltaTime)
{
	if (GetWeaponRound() > 0)
	{
		if (WeaponFiring)
			if (FireTimer < 0.f)
			{
				if (!WeaponReloading)
					Fire();
			}
			else
				FireTimer -= DeltaTime;
	}
	else
	{
		if (!WeaponReloading)
		{
			InitReload();
		}
	}
}

void AWeaponDefault::ReloadTick(float DeltaTime)
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
	if (ShowDebug)
		UE_LOG(LogTemp, Warning, TEXT("Dispersion: MAX = %f. MIN = %f. Current = %f"), CurrentDispersionMax,
	       CurrentDispersionMin, CurrentDispersion);
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

void AWeaponDefault::SetWeaponStateFire(bool bIsFire)
{
	if (CheckWeaponCanFire())
		WeaponFiring = bIsFire;
	else
		WeaponFiring = false;
	FireTimer = 0.01f;
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
	UE_LOG(LogTemp, Warning, TEXT("AWeaponDefault Fire"));
	FireTimer = WeaponSetting.RateOfFire;
	WeaponInfo.Round = WeaponInfo.Round - 1;
	ChangeDispersionByShot();
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), WeaponSetting.SoundFireWeapon,
	                                       ShootLocation->GetComponentLocation());
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WeaponSetting.EffectFireWeapon,
	                                         ShootLocation->GetComponentTransform());
	int8 NumberProjectile = GetNumberProjectileByShot();

	if (ShootLocation)
	{
		FVector SpawnLocation = ShootLocation->GetComponentLocation();
		FRotator SpawnRotation = ShootLocation->GetComponentRotation();
		FProjectileInfo ProjectileInfo;
		ProjectileInfo = GetProjectile();
		FVector EndLocation;
		for (int8 i = 0; i < NumberProjectile; i++) //Shotgun
		{
			EndLocation = GetFireEndLocation();
			 FVector Dir = EndLocation - SpawnLocation;
			 Dir.Normalize();
			SpawnRotation = FMatrix(Dir, FVector(0, 0, 0), FVector(0, 0, 0), FVector::ZeroVector).Rotator();
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
					Projectile->InitProjectile(WeaponSetting.ProjectileSetting);
					UGameplayStatics::FinishSpawningActor(
						Projectile, FTransform(SpawnRotation, SpawnLocation, FVector(1.0f, 1.0f, 1.0f)));
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Projectile is null"));
			}
		}
	}
}


void AWeaponDefault::UpdateStateWeapon(const ECharacterMovementState MovementState)
{
	//ToDo Dispersion
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

void AWeaponDefault::InitReload()
{
	WeaponReloading = true;
	ReloadTimer = WeaponSetting.ReloadTime;

	//ToDo Anim reload
	if (WeaponSetting.AnimCharReload)
		OnWeaponReloadStart.Broadcast(WeaponSetting.AnimCharReload);
}

void AWeaponDefault::FinishReload()
{
	WeaponReloading = false;
	WeaponInfo.Round = WeaponSetting.MaxRound;
	OnWeaponReloadEnd.Broadcast();
}
