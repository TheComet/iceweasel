// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BaseCharacter.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	//Create and set SpringArm component as our RootComponent
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	//Create a Camera Component and attach it to our SpringArm Component "CameraBoom"
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABaseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABaseCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABaseCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABaseCharacter::LookUp);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ABaseCharacter::CrouchButtonReleased);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABaseCharacter::JumpButtonPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ABaseCharacter::JumpButtonReleased);
}

//RPC that is Run on Server
void ABaseCharacter::SetCrouchButtonDown_Implementation(bool IsDown)
{
	bCrouchButtonDown = IsDown;
}
//to validate the RPC function call - this is always required otherwise you get compiler error
bool ABaseCharacter::SetCrouchButtonDown_Validate(bool IsDown)
{
	return true;
}

void ABaseCharacter::SetJumpButtonDown_Implementation(bool IsDown)
{
	bJumpButtonDown= IsDown;
}

bool ABaseCharacter::SetJumpButtonDown_Validate(bool IsDown)
{
	return true;
}


void ABaseCharacter::CrouchButtonPressed()
{
	if (!CanCharacterCrouch())
		return;

	//Set the value locally first
	//If this is server, then the variable will be replicated to everyone else
	bCrouchButtonDown = true;

	//in case this is not the server, then request the server to replicate the variable to everyone else except us (COND_SkipOwner)
	if (!HasAuthority())
		SetCrouchButtonDown(true);
}

void ABaseCharacter::CrouchButtonReleased()
{
	if (!CanCharacterCrouch())
		return;

	bCrouchButtonDown = false;

	if (!HasAuthority())
		SetCrouchButtonDown(false);
}

void ABaseCharacter::JumpButtonPressed()
{
	if (!CanCharacterJump())
		return;

	bJumpButtonDown = true;

	if (!HasAuthority())
		SetJumpButtonDown(true);

	Jump();
}

void ABaseCharacter::JumpButtonReleased()
{
	if (!CanCharacterJump())
		return;

	bJumpButtonDown = false;

	if (!HasAuthority())
		SetJumpButtonDown(false);

	StopJumping();
}


bool ABaseCharacter::CanCharacterCrouch()const
{
	return true;
}

bool ABaseCharacter::CanCharacterJump()const
{
	return CanJump() && !bCrouchButtonDown;
}



void ABaseCharacter::MoveForward(float AxisValue)
{

	if ((Controller != nullptr) && (AxisValue != 0.0f))
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		//get forward vector
		const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(ForwardVector, AxisValue);
	}
}

void ABaseCharacter::MoveRight(float AxisValue)
{
	if ((Controller != nullptr) && (AxisValue != 0.0f))
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		//get right vector
		const FVector RightVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(RightVector, AxisValue);
	}
}


void ABaseCharacter::Turn(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		AddControllerYawInput(AxisValue);
	}
}

void ABaseCharacter::LookUp(float AxisValue)
{
	if (AxisValue != 0.0f)
	{
		AddControllerPitchInput(AxisValue);
	}
}


void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseCharacter, bCrouchButtonDown, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bJumpButtonDown, COND_SkipOwner);
}