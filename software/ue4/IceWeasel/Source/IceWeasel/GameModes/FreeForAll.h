// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseGameMode.h"
#include "FreeForAll.generated.h"

/**
 * 
 */
UCLASS()
class ICEWEASEL_API AFreeForAll : public ABaseGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	
};
