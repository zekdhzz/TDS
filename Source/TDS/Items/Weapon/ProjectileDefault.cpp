#include "ProjectileDefault.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

AProjectileDefault::AProjectileDefault()
{
	PrimaryActorTick.bCanEverTick = true;

	BulletCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Sphere"));
	BulletCollisionSphere->SetSphereRadius(16.f);
	BulletCollisionSphere->bReturnMaterialOnMove = true;
	BulletCollisionSphere->SetCanEverAffectNavigation(false);
	//BulletCollisionSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	RootComponent = BulletCollisionSphere;

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bullet Projectile Mesh"));
	BulletMesh->SetupAttachment(RootComponent);
	BulletMesh->SetCanEverAffectNavigation(false);

	BulletFX = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Bullet FX"));
	BulletFX->SetupAttachment(RootComponent);

	BulletProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Bullet ProjectileMovement"));
	BulletProjectileMovement->UpdatedComponent = RootComponent;
	BulletProjectileMovement->InitialSpeed = 1.f;
	BulletProjectileMovement->MaxSpeed = 0.f;
	BulletProjectileMovement->bRotationFollowsVelocity = true;
	BulletProjectileMovement->bShouldBounce = true;
}

void AProjectileDefault::BeginPlay()
{
	Super::BeginPlay();
	BulletCollisionSphere->OnComponentHit.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereHit);
	BulletCollisionSphere->OnComponentBeginOverlap.AddDynamic(
		this, &AProjectileDefault::BulletCollisionSphereBeginOverlap);
	BulletCollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AProjectileDefault::BulletCollisionSphereEndOverlap);
}

void AProjectileDefault::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectileDefault::InitProjectile(const FProjectileInfo& InitParam, const bool IsDebugMode)
{
	this->SetLifeSpan(InitParam.ProjectileLifeTime);
	BulletProjectileMovement->InitialSpeed = InitParam.ProjectileInitSpeed;
	BulletProjectileMovement->MaxSpeed = InitParam.ProjectileInitSpeed;
	
	if(InitParam.ProjectileStaticMesh)
	{
		BulletMesh->SetStaticMesh(InitParam.ProjectileStaticMesh);
		BulletMesh->SetRelativeTransform(InitParam.ProjectileStaticMeshOffset);
	}
	else
	{
		BulletMesh->DestroyComponent();
	}
	if(InitParam.ProjectileTrailFx)
	{
		BulletFX->SetTemplate(InitParam.ProjectileTrailFx);
		BulletFX->SetRelativeTransform(InitParam.ProjectileTrailFxOffset);
	}
	else
	{
		BulletFX->DestroyComponent();
	}
	
	ProjectileSetting = InitParam;
	ShowDebug = IsDebugMode;
}

void AProjectileDefault::BulletCollisionSphereHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                                                  const FHitResult& Hit)
{
	if (OtherActor && Hit.PhysMaterial.IsValid())
	{
		const EPhysicalSurface Surface = UGameplayStatics::GetSurfaceType(Hit);
		if (ProjectileSetting.HitDecals.Contains(Surface))
		{
			UMaterialInterface* Material = ProjectileSetting.HitDecals[Surface];
			if (Material && OtherComp)
			{
				
				UGameplayStatics::SpawnDecalAttached(Material, FVector(20.0f), OtherComp, NAME_None, Hit.ImpactPoint,
				                                     Hit.ImpactNormal.Rotation(), EAttachLocation::KeepWorldPosition,
				                                     10.0f);
				//UE_LOG(LogTemp, Warning, TEXT("Material %s"), *Material->GetName());
			}
		}
		if (ProjectileSetting.HitFXs.Contains(Surface))
		{
			UParticleSystem* ParticleSystem = ProjectileSetting.HitFXs[Surface];
			if (ParticleSystem)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleSystem,
				                                         FTransform(Hit.ImpactNormal.Rotation(), Hit.ImpactPoint,
				                                                    FVector(1.0f)));
				//UE_LOG(LogTemp, Warning, TEXT("Material %s"), *ParticleSystem->GetName());
			}
		}
		if (ProjectileSetting.HitSound.Contains(Surface))
		{
			USoundBase* HitSound = ProjectileSetting.HitSound[Surface];
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.ImpactPoint);
			//UE_LOG(LogTemp, Warning, TEXT("Material %s"), *HitSound->GetName());
		}
	}
	UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetInstigatorController(), this,
	                              nullptr);
	ImpactProjectile();
	//UGameplayStatics::ApplyRadialDamageWithFalloff()
	//Apply damage cast to if char like bp? //OnAnyTakeDmage delegate
	//UGameplayStatics::ApplyDamage(OtherActor, ProjectileSetting.ProjectileDamage, GetOwner()->GetInstigatorController(), GetOwner(), NULL);
	//or custom damage by health component
}

void AProjectileDefault::BulletCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                           bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectileDefault::BulletCollisionSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AProjectileDefault::ImpactProjectile()
{
	this->Destroy();
}
