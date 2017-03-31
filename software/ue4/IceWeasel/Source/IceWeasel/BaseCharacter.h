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
	//Weapon that player holds in his hands
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;


	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bCrouchButtonDown;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bFireButtonDown;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsSprinting;

	UPROPERTY(Replicated, BlueprintReadWrite)
	bool bIsAimingDownSights;

	//Do others see you (as third person) always aiming down sight on their local screen?
	//if true, your third person character is always aiming down sight regardless of your first person character (first person hands)
	UPROPERTY(EditAnywhere, Category = "Third Person Properties")
	bool bAlwaysADS;

	//[0, 1] - how much to blend between [AimOffsets, Aimoffsets_Ironsights], [BS_Jog, BS_Jog_Ironsights] and [BS_CrouchWalk, BS_CrouchWalk_Ironsights]
	//I interpolate from 0 to 1 to give a smooth tranisition of aiming down sight when bIsAimingDownSights == true
	UPROPERTY(BlueprintReadOnly)
	float ADSBlend;

	//Speed of interpolating ADSBlend i.e Speed of aiming down sight
	UPROPERTY(EditAnywhere, Category = "Player Properties")
	float ADSBlendInterpSpeed;

	//Normal camera FOV
	UPROPERTY(EditAnywhere, Category = "First Person Properties")
	float CameraFOV;

	//Camera FOV when aiming down sights
	UPROPERTY(EditAnywhere, Category = "First Person Properties")
	float ADSCameraFOV;

	//Pitch Rotation of where player is currently looking
	UPROPERTY(Replicated, BlueprintReadWrite)
	float AimPitch;

protected:
	void Sprint(float AxisValue);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetCrouchButtonDown(bool IsDown);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetFireButtonDown(bool IsDown);

	//Unreliable because it's called every frame
	UFUNCTION(Server, UnReliable, WithValidation, BlueprintCallable)
	void ServerSetIsSprinting(bool IsSprinting);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetIsAimingDownSights(bool IsADS);


	UFUNCTION(BlueprintPure)
	bool CanCharacterCrouch()const;

	UFUNCTION(BlueprintPure)
	bool CanCharacterJump()const;

	UFUNCTION(BlueprintPure)
	bool CanCharacterSprint()const;

private:
	void CrouchButtonPressed();
	void CrouchButtonReleased();

	void ADSButtonPressed();
	void ADSButtonReleased();

	void FireButtonPressed();
	void FireButtonReleased();

	//Calculate Pitch to be used inside Animation Blueprint for aimoffsets
	//void CalculatePitch();

	//To be called on Server by Client
	//UFUNCTION(Server, UnReliable, WithValidation)
	//void Server_CalculatePitch();

private:
	float CharacterWalkSpeed;
};
