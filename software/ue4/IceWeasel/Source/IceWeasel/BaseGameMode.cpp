// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BaseGameMode.h"


void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	PlayerControllerArray.Add(NewPlayer);
}

void ABaseGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* PC = Cast<APlayerController>(Exiting);

	if (PC != nullptr)
	{
		PlayerControllerArray.Remove(PC);
	}
}

