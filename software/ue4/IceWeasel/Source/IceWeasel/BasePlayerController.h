// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABasePlayerController();
	
protected:
	virtual void SetupInputComponent() override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

	void JumpButtonDown();
	void JumpButtonReleased();
};
