// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerStart.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

protected:
	ATeamPlayerStart();

	
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Team")
	int TeamNumber;
};
