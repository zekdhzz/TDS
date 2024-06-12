#include "ProjectileDefault_Grenade.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


void AProjectileDefault_Grenade::BeginPlay()
{
	Super::BeginPlay();
}

void AProjectileDefault_Grenade::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimerExplode(DeltaTime);
}

void AProjectileDefault_Grenade::TimerExplode(const float DeltaTime)
{
	if (TimerEnabled)
	{
		if (TimerToExplode > TimeToExplode)
		{
			Explode();
		}
		else
		{
			TimerToExplode += DeltaTime;
		}
	}
}

void AProjectileDefault_Grenade::BulletCollisionSphereHit(class UPrimitiveComponent* HitComp, AActor* OtherActor,
                                                          UPrimitiveComponent* OtherComp, const FVector NormalImpulse,
                                                          const FHitResult& Hit)
{
	Super::BulletCollisionSphereHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileDefault_Grenade::ImpactProjectile()
{
	TimerEnabled = true;
}

void AProjectileDefault_Grenade::Explode()
{
	TimerEnabled = false;
	if (ProjectileSetting.ExplodeFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ProjectileSetting.ExplodeFX, GetActorLocation(),
		                                         GetActorRotation(), FVector(1.0f));
	}
	if (ProjectileSetting.ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ProjectileSetting.ExplodeSound, GetActorLocation());
	}

	if (ShowDebug)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), ProjectileSetting.ProjectileMaxRadiusDamage, 12, FColor::Red,
		                false, 12.0f);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ProjectileSetting.ProjectileMaxRadiusDamage - 
		                (ProjectileSetting.ProjectileMaxRadiusDamage - ProjectileSetting.ProjectileMinRadiusDamage) *
		                0.5f, 12, FColor::Blue, false, 12.0f);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ProjectileSetting.ProjectileMinRadiusDamage, 12, FColor::Green,
		                false, 12.0f);
	}

	// UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), ProjectileSetting.ExplodeMaxDamage,
	//                                                ProjectileSetting.ExplodeMaxDamage * 0.2f, GetActorLocation(),
	//                                                1000.0f, 2000.0f, 5, nullptr, TArray<AActor*>(), nullptr, nullptr);

	this->Destroy();
}
