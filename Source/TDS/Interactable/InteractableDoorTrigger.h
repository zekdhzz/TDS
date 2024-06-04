#pragma once

#include "CoreMinimal.h"
#include "InteractableActor.h"
#include "InteractableDoorTrigger.generated.h"

/**
 * 
 */
UCLASS()
class TDS_API AInteractableDoorTrigger : public AInteractableActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AInteractableDoorTrigger();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	class UBoxComponent* TriggerCapsule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UStaticMeshComponent* DoorMesh;
	
	bool Opening = false;
	bool Closing = false;
	bool IsClosed = true;
	
	FVector DoorInitPosition;
	FVector DoorEndPosition;
	float const DoorOffset = 200.0f;

	FTimerHandle DoorSmoothTimerHandle;
	void SoothingDoorTransformOpen();
	void SoothingDoorTransformClose();
	
	UFUNCTION()
	void OpenDoor();

	UFUNCTION()
	void CloseDoor();

	UFUNCTION()
	void ToggleDoor();

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
