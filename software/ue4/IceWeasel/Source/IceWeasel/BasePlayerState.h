// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "BasePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ABasePlayerState();
	
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void SetTeamNumber(int TeamNumber);

	int GetTeamNumber() const;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetTeamNumber(int TeamNum);

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
	int TeamNum;
	
};
