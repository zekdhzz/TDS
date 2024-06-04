#include "SpawnableParticle.h"

#include "DrawDebugHelpers.h"
#include "Components/SphereComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "TDS/Character/TDSCharacter.h"


ASpawnableParticle::ASpawnableParticle()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));
	CollisionSphere->SetSphereRadius(SphereRadius);
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetHiddenInGame(false);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ASpawnableParticle::OnOverlapBegin);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ASpawnableParticle::OnOverlapEnd);

	Vfx = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	Vfx->SetupAttachment(CollisionSphere);
	Vfx->SetRelativeLocation(FVector::ZeroVector);
	
}

void ASpawnableParticle::BeginPlay()
{
	Super::BeginPlay();
	Vfx->SetTemplate(Cast<UParticleSystem>(StaticLoadObject(UParticleSystem::StaticClass(), this, *EffectTemplate->GetPathName())));
	Vfx->DeactivateSystem();
}

void ASpawnableParticle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASpawnableParticle::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                        const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Error, TEXT("OnOverlapBegin"));
	IsLoop = true;

	if (!VfxSpawnTimerHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("VFXSpawnTimerHandle !IsValid"));
		GetWorldTimerManager().SetTimer(VfxSpawnTimerHandle, this,
		                                &ASpawnableParticle::SpawnParticle,
		                                SpawnRate, true, 0.0f);
	}
}

void ASpawnableParticle::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IsLoop = false;

	UE_LOG(LogTemp, Error, TEXT("OnOverlapEnd"));
}

void ASpawnableParticle::SpawnParticle()
{
	if (IsLoop)
	{
		Vfx->ActivateSystem();
		UE_LOG(LogTemp, Error, TEXT("IsLoop"));
		ATDSCharacter* CurrentPawn = Cast<ATDSCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound,
		                                      CurrentPawn->GetCameraBoom()->GetComponentLocation(), SoundVolume);
	}
	else
	{
		Vfx->DeactivateSystem();
		GetWorldTimerManager().ClearTimer(VfxSpawnTimerHandle);
	}
}
