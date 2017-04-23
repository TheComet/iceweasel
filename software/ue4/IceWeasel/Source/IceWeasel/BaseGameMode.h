// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "BaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API ABaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;


protected:

	UPROPERTY(BlueprintReadOnly)
	TArray<APlayerController*> PlayerControllerArray;

};
