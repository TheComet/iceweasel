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
	CameraBoom->TargetArmLength = 0.0f;
	CameraBoom->bUsePawnControlRotation = true;

	//Create a Camera Component and attach it to our SpringArm Component "CameraSpringArm"
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	//Create a skeletalMesh for holding weapon and attach it to the character mesh
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh());

	FirstPersonViewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonViewMesh"));
	FirstPersonViewMesh->SetupAttachment(Camera);

	//Initialize default values
	ADSBlendInterpSpeed = 10.0f;
	CameraFOV = 120.0f;
	ADSCameraFOV = 90.0f;
	SprintSpeed = 600.0f;

	bAlwaysADS = false;
	bIsInFirstPersonView = true;

}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;

	SetCameraView(bIsInFirstPersonView);
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsSprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		GetCharacterMovement()->MaxFlySpeed = SprintSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = OriginalWalkSpeed;
		GetCharacterMovement()->MaxFlySpeed = OriginalWalkSpeed;
	}

	//can aim down sight on third person
	bool CanAdsOnTP = !bAlwaysADS && !IsLocallyControlled();

	if (!CanAdsOnTP)
	{
		ADSBlend = 1.0f; //1 = fully blend into aiming down sight animation
	}

	if (bIsAimingDownSights)
	{
		if (CanAdsOnTP)
			ADSBlend = FMath::FInterpTo(ADSBlend, 1.0f, DeltaTime, ADSBlendInterpSpeed);
		
		Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, ADSCameraFOV, DeltaTime, ADSBlendInterpSpeed);
	}
	else
	{
		if (CanAdsOnTP)
			ADSBlend = FMath::FInterpTo(ADSBlend, 0.0f, DeltaTime, ADSBlendInterpSpeed);
		
		Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, CameraFOV, DeltaTime, ADSBlendInterpSpeed);
	}

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Sprint", this, &ABaseCharacter::Sprint);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABaseCharacter::DoCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ABaseCharacter::DoUnCrouch);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &ABaseCharacter::ADSButtonPressed);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &ABaseCharacter::ADSButtonReleased);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABaseCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABaseCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("SwitchView", IE_Pressed, this, &ABaseCharacter::ChangeView);
}

bool ABaseCharacter::CanCharacterCrouch()const
{
	return !GetCharacterMovement()->IsFalling();
}

bool ABaseCharacter::CanCharacterJump()const
{
	return CanJump() && !bIsCrouched;
}

bool ABaseCharacter::CanCharacterSprint()const
{
	FVector ForwardVector = GetActorForwardVector(); //normalized forward direction of the player
	FVector VelocityVector = GetCharacterMovement()->Velocity.GetSafeNormal(); //the normalized direction in which the player is moving
	
	bool IsMovingForward = false;
	bool IsMovingOnRightVector = false;

	float p = FVector::DotProduct(ForwardVector, VelocityVector); //cosine of angle between forward vector and velocity vector

	//p = 1 if player is moving forward
	//p = -1 if player is moving backward
	//p = 0 if player is moving right or left

	//we don't get exact values due to limited precision so check if p is nearly equal to 1, -1 or 0

	if (p > 0.7f) //check if dot product is nearly one
		IsMovingForward = true;

	if (p < 0.1f) //check if dot product is nearly zero
		IsMovingOnRightVector = true;
	

	return !bIsCrouched && //Is not crouching
		!GetCharacterMovement()->IsFalling() && //Is not Falling
		(GetCharacterMovement()->Velocity.SizeSquared() != 0.0f) && //Is not sationary
		IsMovingForward && //Is moving forward and not backward
		!IsMovingOnRightVector; //Is NOT moving right or left
	
}

FRotator ABaseCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();


	return AimRotLS;
}

void ABaseCharacter::Sprint(float AxisValue)
{

	if (AxisValue != 0.0f && CanCharacterSprint())
	{
		bIsSprinting = true;

		if (!HasAuthority())
			ServerSetIsSprinting(true);
	}
	else
	{
		bIsSprinting = false;

		if (!HasAuthority())
			ServerSetIsSprinting(false);
	}
}

void ABaseCharacter::DoCrouch()
{
	if (!CanCharacterCrouch())
		return;

	Crouch();
}


void ABaseCharacter::DoUnCrouch()
{
	UnCrouch();
}

void ABaseCharacter::SetCameraView(bool bFirstPerson)
{
	if (bFirstPerson)
	{

		GetMesh()->SetOwnerNoSee(true);
		FirstPersonViewMesh->SetOnlyOwnerSee(true);
		FirstPersonViewMesh->SetOwnerNoSee(false);

		WeaponMesh->SetOnlyOwnerSee(false);
		WeaponMesh->SetOwnerNoSee(false);

		CameraBoom->TargetArmLength = 0.0f;

		bIsInFirstPersonView = true;
	}
	else
	{
		GetMesh()->SetOwnerNoSee(false);
		FirstPersonViewMesh->SetOnlyOwnerSee(false);
		FirstPersonViewMesh->SetOwnerNoSee(true);

		WeaponMesh->SetOnlyOwnerSee(false);
		WeaponMesh->SetOwnerNoSee(true);

		CameraBoom->TargetArmLength = 300.0f;

		bIsInFirstPersonView = false;
	}
}


//Replicate variables
void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABaseCharacter, bFireButtonDown, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bIsSprinting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ABaseCharacter, bIsAimingDownSights, COND_SkipOwner);
}



#pragma region Server RPCs
//RPC that is Run on Server
void ABaseCharacter::ServerSetFireButtonDown_Implementation(bool IsDown)
{
	bFireButtonDown = IsDown;
}

bool ABaseCharacter::ServerSetFireButtonDown_Validate(bool IsDown)
{
	return true;
}

//RPC that is Run on Server
void ABaseCharacter::ServerSetIsSprinting_Implementation(bool IsSprinting)
{
	bIsSprinting = IsSprinting;
}

bool ABaseCharacter::ServerSetIsSprinting_Validate(bool IsSprinting)
{
	return true;
}

//RPC that is Run on Server
void ABaseCharacter::ServerSetIsAimingDownSights_Implementation(bool IsADS)
{
	bIsAimingDownSights = IsADS;
}

bool ABaseCharacter::ServerSetIsAimingDownSights_Validate(bool IsADS)
{
	return true;
}
#pragma endregion 


#pragma region Action Mapping

void ABaseCharacter::ADSButtonPressed()
{
	bIsAimingDownSights = true;

	if (!HasAuthority())
		ServerSetIsAimingDownSights(bIsAimingDownSights);
}

void ABaseCharacter::ADSButtonReleased()
{
	bIsAimingDownSights = false;

	if (!HasAuthority())
		ServerSetIsAimingDownSights(bIsAimingDownSights);
}

void ABaseCharacter::FireButtonPressed()
{
	bFireButtonDown = true;

	if (!HasAuthority())
		ServerSetFireButtonDown(bFireButtonDown);
}

void ABaseCharacter::FireButtonReleased()
{
	bFireButtonDown = false;

	if (!HasAuthority())
		ServerSetFireButtonDown(bFireButtonDown);
}

void ABaseCharacter::ChangeView()
{
	SetCameraView(!bIsInFirstPersonView);
}
#pragma endregion

