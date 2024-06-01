#include "InteractableDoorTrigger.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"


AInteractableDoorTrigger::AInteractableDoorTrigger()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	TriggerCapsule = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerCapsule"));
	TriggerCapsule->SetCollisionProfileName(TEXT("Trigger"));
	RootComponent = TriggerCapsule;

	TriggerCapsule->OnComponentBeginOverlap.AddDynamic(this, &AInteractableDoorTrigger::OnOverlapBegin);
	TriggerCapsule->OnComponentEndOverlap.AddDynamic(this, &AInteractableDoorTrigger::OnOverlapEnd);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DoorAsset(
		TEXT("/Game/Enviroment/Meshes/Decos/m_deco_door_C.m_deco_door_C"));

	if (DoorAsset.Object)
	{
		DoorMesh->SetStaticMesh(DoorAsset.Object);
	}

	DoorInitPosition = DoorMesh->GetRelativeLocation();
}

void AInteractableDoorTrigger::BeginPlay()
{
	Super::BeginPlay();

	//DrawDebugBox(GetWorld(), GetActorLocation(), BoxComp->GetScaledBoxExtent(), FQuat(GetActorRotation()), FColor::Turquoise, true, 999, 0, 2);
}

void AInteractableDoorTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractableDoorTrigger::SoothingDoorTransformOpen()
{
	if (!FMath::IsNearlyEqual(DoorMesh->GetRelativeLocation().Z, DoorInitPosition.Z - DoorOffset, 0.5f))
	{
		float EndLocationStep = FMath::FInterpTo(DoorMesh->GetRelativeLocation().Z, DoorInitPosition.Z - DoorOffset,
		                                         GetWorld()->GetDeltaSeconds(), 10.0f);
		
		DoorMesh->SetRelativeLocation(FVector(DoorMesh->GetRelativeLocation().X, DoorMesh->GetRelativeLocation().Y,
		                                      EndLocationStep));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClearTimer"));
		GetWorldTimerManager().ClearTimer(DoorSmoothTimerHandle);
	}
}

void AInteractableDoorTrigger::SoothingDoorTransformClose()
{
	if (!FMath::IsNearlyEqual(DoorMesh->GetRelativeLocation().Z, DoorInitPosition.Z, 0.5f))
	{
		float EndLocationStep = FMath::FInterpTo(DoorMesh->GetRelativeLocation().Z, DoorInitPosition.Z,
												 GetWorld()->GetDeltaSeconds(), 10.0f);
		
		DoorMesh->SetRelativeLocation(FVector(DoorMesh->GetRelativeLocation().X, DoorMesh->GetRelativeLocation().Y,
											  EndLocationStep));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ClearTimer"));
		GetWorldTimerManager().ClearTimer(DoorSmoothTimerHandle);
	}
}

void AInteractableDoorTrigger::OpenDoor()
{
	if (!DoorSmoothTimerHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("DoorSmoothTimerHandle !IsValid"));
		GetWorldTimerManager().SetTimer(DoorSmoothTimerHandle, this,
		                                &AInteractableDoorTrigger::SoothingDoorTransformOpen,
		                                GetWorld()->GetDeltaSeconds(), true, 0.0f);
	}
}

void AInteractableDoorTrigger::CloseDoor()
{
	UE_LOG(LogTemp, Error, TEXT("CloseDoor"));

	if (!DoorSmoothTimerHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("DoorSmoothTimerHandle !IsValid"));
		GetWorldTimerManager().SetTimer(DoorSmoothTimerHandle, this,
		                                &AInteractableDoorTrigger::SoothingDoorTransformClose,
		                                GetWorld()->GetDeltaSeconds(), true, 0.0f);
	}
}

void AInteractableDoorTrigger::ToggleDoor()
{
	if (IsClosed)
	{
		// door is opened
		Opening = true;
		Closing = false;
		IsClosed = false;
	}
	else
	{
		// door is closed
		Opening = false;
		Closing = true;
		IsClosed = true;
	}
}

void AInteractableDoorTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                              const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Error, TEXT("OnOverlapBegin"));
	ToggleDoor();
	if (Opening)
	{
		OpenDoor();
	}

	if (Closing)
	{
		CloseDoor();
	}
}

void AInteractableDoorTrigger::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Error, TEXT("OnOverlapEnd"));
	ToggleDoor();
	if (Opening)
	{
		OpenDoor();
	}

	if (Closing)
	{
		CloseDoor();
	}
}
