#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TDS/Enumerations/TDSCharacterEnums.h"
#include "TDS/Structs/TDSWeaponStructs.h"
#include "WeaponDefault.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponFireStart, UAnimMontage*, AnimFireChar);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponReloadStart, UAnimMontage*, Anim);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponReloadEnd);

UCLASS()
class TDS_API AWeaponDefault : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponDefault();

	FOnWeaponFireStart OnWeaponFireStart;
	FOnWeaponReloadEnd OnWeaponReloadEnd;
	FOnWeaponReloadStart OnWeaponReloadStart;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USceneComponent* SceneComponent = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class USkeletalMeshComponent* SkeletalMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UStaticMeshComponent* StaticMeshWeapon = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = Components)
	class UArrowComponent* ShootLocation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
	FWeaponInfo WeaponSetting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Info")
	FAdditionalWeaponInfo WeaponInfo;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void FireTick(float DeltaTime);
	void ReloadTick(float DeltaTime);
	void DispersionTick(float DeltaTime);

	void WeaponInit();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FireLogic")
	bool WeaponFiring = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	bool WeaponReloading = false;
	bool WeaponAiming = false;
	void UpdateWeaponAimingState(bool bIsAiming);
	UFUNCTION(BlueprintCallable)
	void SetWeaponStateFire(bool bIsFire);
	void UpdateStateWeapon(ECharacterMovementState MovementState);
	bool CheckWeaponCanFire() const;

	FProjectileInfo GetProjectile();
	void Fire();

	void ChangeDispersionByShot();
	float GetCurrentDispersion() const;
	FVector ApplyDispersionToShoot(FVector DirectionShoot) const;

	FVector GetFireEndLocation() const;
	int8 GetNumberProjectileByShot() const;

	//Timers
	float FireTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic")
	float ReloadTimer = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReloadLogic Debug") //Remove !!! Debug
	float ReloadTime = 0.0f;

	//flags
	bool BlockFire = false;
	//Dispersion
	bool ShouldReduceDispersion = false;
	float CurrentDispersion = 0.0f;
	float CurrentDispersionMax = 1.0f;
	float CurrentDispersionMin = 0.1f;
	float CurrentDispersionRecoil = 0.1f;
	float CurrentDispersionReduction = 0.1f;

	FVector ShootEndLocation = FVector(0);

	UFUNCTION(BlueprintCallable)
	int32 GetWeaponRound() const;
	void InitReload();
	void FinishReload();
	UFUNCTION()
	void AnimWeaponStart(UAnimMontage* WeaponAnim) const;

	bool DropShellFlag = false;
	float DropShellTimer = -1.0f;
	UFUNCTION()
	void InitDropMesh(UStaticMesh* DropMesh, const FTransform& Offset, FVector DropImpulseDirection, float LifeTimeMesh,
	                  float ImpulseRandomDispersion, float PowerImpulse, float CustomMass);
	void ShellDropTick(float DeltaTime);
	UFUNCTION()
	void ShellDropFire(UStaticMesh* DropMesh, const FTransform& Offset, FVector DropImpulseDirection,
	                   float LifeTimeMesh, float ImpulseRandomDispersion, float PowerImpulse, float CustomMass,
	                   FVector LocalDir);

	bool DropClipFlag = false;
	float DropClipTimer = -1.0;
	void ClipDropTick(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool ShowDebug = false;
	UFUNCTION(BlueprintCallable)
	bool GetDebugState() const;
	UFUNCTION(BlueprintCallable)
	void SetDebugState(bool IsDebugMode);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float SizeVectorToChangeShootDirectionLogic = 100.0f;

	UFUNCTION()
	static void SpawnTraceHitDecal(UMaterialInterface* DecalMaterial, const FHitResult& HitResult);
	UFUNCTION()
	void SpawnTraceHitFX(UParticleSystem* FxTemplate, const FHitResult& HitResult) const;
	UFUNCTION()
	void SpawnTraceHitSound(USoundBase* HitSound, const FHitResult& HitResult) const;

	void DebugShowStatus() const;
};
