#pragma once

#include "CoreMinimal.h"
#include "SpawnableActor.h"
#include "SpawnableParticle.generated.h"

/**
 * 
 */
UCLASS()
class TDS_API ASpawnableParticle : public ASpawnableActor
{
	GENERATED_BODY()

public:
	ASpawnableParticle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY()
	class USphereComponent* CollisionSphere;
	UPROPERTY()
	UParticleSystemComponent* Vfx;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category="VFX Params")
	UParticleSystem* EffectTemplate;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"), Category="VFX Params")
	USoundBase* Sound;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category="VFX Params")
	float SphereRadius = 800.0f;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category="VFX Params")
	float SpawnRate = 2.0f;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category="VFX Params")
	float SoundVolume = 1.0f;
	
	bool IsLoop;
	FTimerHandle VfxSpawnTimerHandle;
	void SpawnParticle();
};
