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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	//Spring arm component to hold and position the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bCrouchButtonDown;

	UPROPERTY(Replicated)
	bool bJumpButtonDown;

protected:
	//WSAD input movements
	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);

	//Mouse input movements
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	//To be called on Server by Client
	UFUNCTION(Server, Reliable, WithValidation)
	void SetCrouchButtonDown(bool IsDown);

	//To be called on Server by Client
	UFUNCTION(Server, Reliable, WithValidation)
	void SetJumpButtonDown(bool IsDown);

	
private:
	void CrouchButtonPressed();
	void CrouchButtonReleased();

	void JumpButtonPressed();
	void JumpButtonReleased();


	bool CanCharacterCrouch()const;
	bool CanCharacterJump()const;
	
};
