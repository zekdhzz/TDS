#pragma once

#include "CoreMinimal.h"
#include "ProjectileDefault.h"
#include "ProjectileDefault_Grenade.generated.h"

UCLASS()
class TDS_API AProjectileDefault_Grenade : public AProjectileDefault
{
	GENERATED_BODY()

public:
	AProjectileDefault_Grenade();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void Explode();
	void TimerExplode(float DeltaTime);
	virtual void ImpactProjectile() override;
	virtual void BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	                                      UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	                                      const FHitResult& Hit) override;
	bool TimerEnabled = false;
	float TimerToExplode = 0.0f;
	float TimeToExplode = 5.0f;
};
