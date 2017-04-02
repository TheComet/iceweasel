// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BasePlayerController.h"


ABasePlayerController::ABasePlayerController()
{
	//Initialize variables to default value
	AimPitch = 0.0f;
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

#pragma region Server RPCs
/*
void ABasePlayerController::ServerCalculatePitch_Implementation()
{
	CalculatePitch();
}


bool ABasePlayerController::ServerCalculatePitch_Validate()
{
	return true;
}
*/
#pragma endregion


void ABasePlayerController::MoveForward(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		//get forward vector
		const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		
		GetPawn()->AddMovementInput(ForwardVector, AxisValue);
	}
}

void ABasePlayerController::MoveRight(float AxisValue)
{
	if (AxisValue != 0.0f)
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

		CalculatePitch();

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


//Calculate AimPitch to be used inside animation blueprint for aimoffsets
void ABasePlayerController::CalculatePitch()
{
	APawn* p = GetPawn();

	if (p == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn is null."));
		return;
	}
	

	FRotator ActorRotation = p->GetActorRotation();

	FRotator Delta = ControlRotation - ActorRotation;

	FRotator Pitch(AimPitch, 0.0f, 0.0f);

	FRotator Final = FMath::RInterpTo(Pitch, Delta, GetWorld()->DeltaTimeSeconds, 0.0f);


	AimPitch = FMath::ClampAngle(Final.Pitch, -90.0f, 90.0f);


}

/*
void ABasePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	//DOREPLIFETIME_CONDITION(ABasePlayerController, AimPitch, COND_SkipOwner);
}
*/