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
	
	UFUNCTION(BlueprintCallable)
	void SetTeamId(int Id);

	FORCEINLINE INT GetTeamId()const;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetTeamId(int Id);

protected:
	UPROPERTY(Replicated, BlueprintReadOnly)
	int TeamId;
	
};
