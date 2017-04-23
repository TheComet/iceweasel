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
	//Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	//Spring arm component to hold and position the camera (for third person)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	//Weapon that player holds in his hands
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonViewMesh;

	//Is this player firing a weapon
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Character|Player Status")
	bool bFireButtonDown;

	//Is this player sprinting
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Character|Player Status")
	bool bIsSprinting;

	//Is this player aiming down sight
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Character|Player Status")
	bool bIsAimingDownSights;

	UPROPERTY(BlueprintReadOnly, Category = "Character|Player Status")
	bool bIsInFirstPersonView;

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

	//Speed of the player while sprinting
	UPROPERTY(EditAnywhere, Category = "Player Properties")
	float SprintSpeed;

	//Normal camera FOV
	UPROPERTY(EditAnywhere, Category = "First Person Properties")
	float CameraFOV;

	//Camera FOV when aiming down sights
	UPROPERTY(EditAnywhere, Category = "First Person Properties")
	float ADSCameraFOV;


protected:
	void Sprint(float AxisValue);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Character|Server RPCs")
	void ServerSetFireButtonDown(bool IsDown);

	//Unreliable because it's called every frame
	UFUNCTION(Server, UnReliable, WithValidation, BlueprintCallable, Category = "Character|Server RPCs")
	void ServerSetIsSprinting(bool IsSprinting);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Character|Server RPCs")
	void ServerSetIsAimingDownSights(bool IsADS);

	UFUNCTION(BlueprintPure, Category = "Character|Player Status")
	bool CanCharacterCrouch()const;

	UFUNCTION(BlueprintPure, Category = "Character|Player Status")
	bool CanCharacterJump()const;

	UFUNCTION(BlueprintPure, Category = "Character|Player Status")
	bool CanCharacterSprint()const;

	UFUNCTION(BlueprintPure, Category = "Character|Player Status")
	FRotator GetAimOffsets()const;

	UFUNCTION(BlueprintCallable, Category = "Character|Player Status")
	void SetCameraView(bool bFirstPerson);

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void DoCrouch();

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void DoUnCrouch();


private:
	void ADSButtonPressed();
	void ADSButtonReleased();

	void FireButtonPressed();
	void FireButtonReleased();

	/** Switches the view between first person and third person */
	void ChangeView();

private:
	//Store the original WalkSpeed from CharacterMovement so that we can use it later
	float OriginalWalkSpeed;
};
