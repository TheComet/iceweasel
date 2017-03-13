// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class ICEWEASEL_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	//ThirdPerson Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	//Spring arm component to hold and position the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bCrouchButtonDown;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bFireButtonDown;

	UPROPERTY(Replicated)
	bool bJumpButtonDown;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsSprinting;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsAimingDownSights;

	//[0, 1] - how much to blend between [AimOffsets, Aimoffsets_Ironsights], [BS_Jog, BS_Jog_Ironsights] and [BS_CrouchWalk, BS_CrouchWalk_Ironsights]
	//I interpolate from 0 to 1 to give a smooth tranisition of aiming down sight when bIsAimingDownSights == true
	UPROPERTY(BlueprintReadOnly)
	float ADSBlend;

	//Speed of interpolating ADSBlend 
	UPROPERTY(EditAnywhere, Category = PlayerProperties)
	float ADSBlendInterpSpeed;

	UPROPERTY(EditAnywhere, Category = PlayerProperties)
	float CameraFOV;

	UPROPERTY(EditAnywhere, Category = PlayerProperties)
	float ADSCameraFOV;

	UPROPERTY(ReplicatedUsing = OnRep_AimPitch, BlueprintReadWrite)
	float AimPitch;

	UPROPERTY(BlueprintReadOnly)
	float SmoothAimPitch;

protected:
	//WSAD input movements
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);

	//Mouse input movements
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	void Sprint(float AxisValue);

	//To be called on Server by Client
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SetCrouchButtonDown(bool IsDown);

	//To be called on Server by Client
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SetJumpButtonDown(bool IsDown);

	//To be called on Server by Client
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SetFireButtonDown(bool IsDown);

	//To be called on Server by Client - Unreliable because I call it every frame
	UFUNCTION(Server, UnReliable, WithValidation, BlueprintCallable)
	void SetIsSprinting(bool IsSprinting);

	//To be called on Server by Client
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void SetIsAimingDownSights(bool IsADS);


	UFUNCTION()
	void OnRep_AimPitch(float oldValue);

	UFUNCTION(BlueprintPure)
	inline bool CanCharacterCrouch()const;
	UFUNCTION(BlueprintPure)
	inline bool CanCharacterJump()const;
	UFUNCTION(BlueprintPure)
	inline bool CanCharacterSprint()const;

private:
	void CrouchButtonPressed();
	void CrouchButtonReleased();

	void JumpButtonPressed();
	void JumpButtonReleased();

	void ADSButtonPressed();
	void ADSButtonReleased();

	void FireButtonPressed();
	void FireButtonReleased();

	//Calculate Pitch to be used inside Animation Blueprint for aimoffsets
	void CalculatePitch();

	//To be called on Server by Client
	UFUNCTION(Server, UnReliable, WithValidation)
	void Server_CalculatePitch();


private:
	float CharacterWalkSpeed;
};
