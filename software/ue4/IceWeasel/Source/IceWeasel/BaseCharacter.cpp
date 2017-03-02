// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BaseCharacter.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);
	
	//Create and set SpringArm component as our RootComponent
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	//Create a Camera Component and attach it to our SpringArm Component "CameraBoom"
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	AimPitch = 0.0f;
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
	PlayerInputComponent->BindAxis("Sprint", this, &ABaseCharacter::Sprint);
	
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

bool ABaseCharacter::SetCrouchButtonDown_Validate(bool IsDown)
{
	return true;
}
//RPC that is Run on Server
void ABaseCharacter::SetJumpButtonDown_Implementation(bool IsDown)
{
	bJumpButtonDown = IsDown;
}

bool ABaseCharacter::SetJumpButtonDown_Validate(bool IsDown)
{
	return true;
}

//RPC that is Run on Server
void ABaseCharacter::SetIsSprinting_Implementation(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
}

bool ABaseCharacter::SetIsSprinting_Validate(bool IsSprinting)
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
	return !GetCharacterMovement()->IsFalling();
}

bool ABaseCharacter::CanCharacterJump()const
{
	return CanJump() && !bCrouchButtonDown;
}

bool ABaseCharacter::CanCharacterSprint()const
{
	FVector ForwardVector = GetActorForwardVector().SafeNormal(); //forward direction of the player
	FVector VelocityVector = GetCharacterMovement()->Velocity.SafeNormal(); //the direction in which the player is moving
	
	bool IsMovingForward = false;
	bool IsMovingRight = false;

	float p = FVector::DotProduct(ForwardVector, VelocityVector);

	if (p > 0.7f) //is dot product positive i.e cosine of angle between forward vector and velocity vector
		IsMovingForward = true;

	if (p < 0.1f)
		IsMovingRight = true;
	

	return !bCrouchButtonDown && //Is not crouching
		!GetCharacterMovement()->IsFalling() && //Is not Falling
		(GetCharacterMovement()->Velocity.SizeSquared() != 0.0f) && //Is not sationary
		IsMovingForward &&
		!IsMovingRight;
}

void ABaseCharacter::Sprint(float AxisValue)
{

	if (AxisValue != 0.0f && //is sprint button down
		 CanCharacterSprint() //make sure it's in valid state to sprint. Example - you shouldn't be able to sprint while crouching
		)
	{
		bIsSprinting = true;

		if (!HasAuthority())
			SetIsSprinting(true);
	}
	else
	{
		bIsSprinting = false;

		if (!HasAuthority())
			SetIsSprinting(false);


	}
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
		if (HasAuthority()) //if server
		{
			CalculatePitch();  //calculate the pitch and send it to all other clients
		}
		else //if client
		{
			CalculatePitch(); //first do it locally
			Server_CalculatePitch(); //then let others get our updated pitch
		}

		AddControllerPitchInput(AxisValue);
	}
}

//RPC that is Run on Server
void ABaseCharacter::Server_CalculatePitch_Implementation()
{
	CalculatePitch();
}

bool ABaseCharacter::Server_CalculatePitch_Validate()
{
	return true;
}
//Calculate AimPitch to be used inside animation blueprint for aimoffsets
void ABaseCharacter::CalculatePitch()
{
	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator ActorRotation = GetActorRotation();

	FRotator Delta = ControlRotation - ActorRotation;

	FRotator Pitch(AimPitch, 0.0f, 0.0f);

	FRotator Final =FMath::RInterpTo(Pitch, Delta, GetWorld()->DeltaTimeSeconds, 0.0f);

	AimPitch = FMath::ClampAngle(Final.Pitch, -90.0f, 90.0f);
}

//Replicate variables
void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseCharacter, bCrouchButtonDown, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bJumpButtonDown, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bIsSprinting, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ABaseCharacter, AimPitch, COND_SkipOwner);
}