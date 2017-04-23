// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "BaseSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API UBaseSaveGame : public USaveGame
{
	GENERATED_BODY()

public:


	UPROPERTY(BlueprintReadWrite)
	int SelectedCharacterIndex;
	
};
