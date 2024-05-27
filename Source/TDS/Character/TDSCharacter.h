// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDS/FuncLibrary/TDSTypes.h"
#include "TDSCharacter.generated.h"

UCLASS(Blueprintable)
class ATDSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATDSCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraMinHeight = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraMaxHeight = 1500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraZoomDistance = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraZoomSmoothness = 7.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraSensitivity = 125.0f;
	UFUNCTION(BlueprintCallable)
	void CameraZoomInput(float Value);
	UFUNCTION()
	void SoothingCameraZoom();
	FTimerHandle CameraSmoothTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	EMovementState MovementState = EMovementState::Run_State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	FCharacterSpeed MovementInfo;
	float AxisX = 0.0f;
	float AxisY = 0.0f;

	UFUNCTION()
	void InputAxisX(float Value);
	UFUNCTION()
	void InputAxisY(float Value);

	float CharacterMaxMovementSpeed;

	UFUNCTION()
	void MovementTick();
	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();
	UFUNCTION(BlueprintCallable)
	void ChangeMovementState(EMovementState NewMovementState);

	UFUNCTION(BlueprintCallable)
	void CharacterAim();
	UFUNCTION(BlueprintCallable)
	void CharacterWalk();
	UFUNCTION(BlueprintCallable)
	void CharacterRun();
	UFUNCTION(BlueprintCallable)
	void CharacterSprint();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Movement")
	bool IsAiming;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Movement")
	bool IsWalking;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Movement")
	bool IsRunning;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Movement")
	bool IsSprinting;
	bool IsMovingInDirectionToInput;

	void PrintState() const;

	// UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Movement")
	// FVector MousePositionVector;

	float MaxStamina = 100.f;
	float Stamina = MaxStamina;
	float RecoveryStaminaPerTick = 0.5f;
	float SpendStaminaPerTick = 0.1f;
	bool IsStaminaRecovering;

	void RecoveryStamina();
	FTimerHandle StaminaRecoveryTimerHandle;
};
