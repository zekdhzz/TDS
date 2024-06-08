// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TDS/Enumerations/TDSCharacterEnums.h"
#include "TDS/Structs/TDSCharacterStructs.h"
#include "TDSCharacter.generated.h"

class AWeaponDefault;

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

protected:
	virtual void BeginPlay() override;
private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraMinHeight = 1500.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	float CameraMaxHeight = 3000.0f;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	FVector CursorSize = FVector(20.0f, 40.0f, 40.0f);
	UDecalComponent* CurrentCursor = nullptr;
	UFUNCTION(BlueprintCallable)
	UDecalComponent* GetCursorToWorld() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	ECharacterMovementState MovementState = ECharacterMovementState::Run_State;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	FCharacterSpeed MovementInfo;
	float CharacterMaxMovementSpeed;
	float AxisX = 0.0f;
	float AxisY = 0.0f;
	UFUNCTION()
	
	void InputAxisX(float Value);
	UFUNCTION()
	void InputAxisY(float Value);
	UFUNCTION()
	void InputAttackPressed();
	UFUNCTION()
	void InputAttackReleased();

	UFUNCTION()
	void MovementTick();
	UFUNCTION(BlueprintCallable)
	void CharacterUpdate();
	UFUNCTION(BlueprintCallable)
	void ChangeMovementState(ECharacterMovementState NewMovementState);

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

	float MaxStamina = 100.f;
	float Stamina = MaxStamina;
	float RecoveryStaminaPerTick = 0.5f;
	float SpendStaminaPerTick = 0.1f;
	bool IsStaminaRecovering;
	FTimerHandle StaminaRecoveryTimerHandle;
	void RecoveryStamina();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	TSubclassOf<AWeaponDefault> InitWeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	FName InitWeaponName;
	
	UFUNCTION(BlueprintCallable)
	void AttackCharEvent(bool bIsFiring);
	AWeaponDefault* CurrentWeapon = nullptr;
	UFUNCTION(BlueprintCallable)
	AWeaponDefault* GetCurrentWeapon() const;
	UFUNCTION(BlueprintCallable)
	void InitWeapon(FName IdWeaponName);
	UFUNCTION(BlueprintCallable)
	void TryReloadWeapon();
	UFUNCTION()
	void WeaponReloadStart(UAnimMontage* Anim);
	UFUNCTION()
	void WeaponReloadEnd();
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadStart_BP(UAnimMontage* Anim);
	UFUNCTION(BlueprintNativeEvent)
	void WeaponReloadEnd_BP();
	void SetWeaponDisplacement(FVector_NetQuantize Location) const;

	//for debug
	void PrintState() const;
};
