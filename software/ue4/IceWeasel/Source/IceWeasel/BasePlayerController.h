// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "BasePlayerController.generated.h"

/**
 * PlayerController is what controls the character
 * Input code can be in PlayerController aswell as in Character but if the game allows the player to like switch characters 
 * then it's better to write input in PlayerController
 * A PlayerController can Posses or UnPosses a pawn and it can only Posses one Pawn at a time
 */


UCLASS()
class ICEWEASEL_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ABasePlayerController();
	
protected:

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);
	void Turn(float AxisValue);
	void LookUp(float AxisValue);

private:
	void JumpButtonDown();
	void JumpButtonReleased();


public:
	/** A Choosen Character to spawn from this Character Array and posses when this player is connected to a server */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, DisplayName="Player Character", Category = "Player Character")
	TArray<TSubclassOf<ACharacter>> SelectedCharacter;

	/** Which Character to Spawn */
	UPROPERTY(BlueprintReadWrite)
	int SelectedCharacterIndex;
};
