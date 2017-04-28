// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseGameMode.h"
#include "TeamDeathMatch.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API ATeamDeathMatch : public ABaseGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Choose a team for newly joined player - true if successful */
	bool ChooseTeam(APlayerController* Player);

private:
	TArray<AActor*> PlayerStarts;
};
