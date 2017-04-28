// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BasePlayerController.h"
#include "BaseSaveGame.h"


ABasePlayerController::ABasePlayerController()
{
	//Initialize defaults
}

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("BasePlayerController BeginPlay!."));

	SelectedCharacterIndex = 0;
}


void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &ABasePlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABasePlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &ABasePlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &ABasePlayerController::LookUp);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ABasePlayerController::JumpButtonDown);
	InputComponent->BindAction("Jump", IE_Released, this, &ABasePlayerController::JumpButtonReleased);
}


void ABasePlayerController::MoveForward(float AxisValue)
{
	if (AxisValue != 0.0f && GetPawn())
	{
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		//get forward vector
		const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		GetPawn()->AddMovementInput(ForwardVector, AxisValue);
	}
}

void ABasePlayerController::MoveRight(float AxisValue)
{
	if (AxisValue != 0.0f && GetPawn())
	{
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		//get right vector
		const FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		GetPawn()->AddMovementInput(RightVector, AxisValue);
	}
}

void ABasePlayerController::LookUp(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		AddPitchInput(AxisValue);
	}
}


void ABasePlayerController::Turn(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		AddYawInput(AxisValue);
	}
}

void ABasePlayerController::JumpButtonDown()
{
	ACharacter* character = GetCharacter();

	if (character)
	{
		character->Jump();
	}
}

void ABasePlayerController::JumpButtonReleased()
{
	ACharacter* character = GetCharacter();

	if (character)
	{
		character->StopJumping();
	}
}
