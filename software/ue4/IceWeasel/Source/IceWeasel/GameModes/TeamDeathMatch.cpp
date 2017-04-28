// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "TeamDeathMatch.h"
#include "BasePlayerState.h"
#include "BasePlayerController.h"
#include "TeamPlayerStart.h"


void ATeamDeathMatch::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);


	if (!ChooseTeam(NewPlayer))
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("ChooseTeam() failed!"));
		return;
	}

	//Loop through every TeamPlayerStart in the world and save it to an array
	for (TActorIterator<ATeamPlayerStart> PlayerStartItr(GetWorld()); PlayerStartItr; ++PlayerStartItr)
	{
		ATeamPlayerStart* TPS = *PlayerStartItr;

		PlayerStarts.AddUnique(TPS);
	}

	TArray<FTransform> TeamASpawns;
	TArray<FTransform> TeamBSpawns;


	if (PlayerStarts.Num() > 0)
	{
		//Loop through all PlayerStarts
		for (int32 i = 0; i < PlayerStarts.Num(); ++i)
		{
			ATeamPlayerStart* TeamPlayerStart = Cast<ATeamPlayerStart>(PlayerStarts[i]);

			//Collect TEAM_A PlayerStarts Transform into a separate array
			if (TeamPlayerStart->TeamNumber == TEAM_A)
				TeamASpawns.Add(TeamPlayerStart->GetTransform());

			//Collect TEAM_B PlayerStarts Transform into a separate array
			if (TeamPlayerStart->TeamNumber == TEAM_B)
				TeamBSpawns.Add(TeamPlayerStart->GetTransform());
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("No TeamPlayerStart available."));
		return;
	}

	ABasePlayerController* PC = Cast<ABasePlayerController>(NewPlayer);
	ABasePlayerState* PState = Cast<ABasePlayerState>(NewPlayer->PlayerState);


	GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, FString::Printf(TEXT("Charcter : %i"), PC->SelectedCharacterIndex));


	if (!PC->SelectedCharacter.IsValidIndex(0))
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("No Characters available in Charactar Array."));
		return;
	}


	if (PC && PState)
	{
		if (PC->SelectedCharacter[PC->SelectedCharacterIndex])
		{
			//UnPosses and destroy the default pawn spawned by GameMode if there is one
			if (PC->GetPawn())
			{
				PC->GetPawn()->Destroy();
				PC->UnPossess();
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("UnPossed and Destroying Existing pawn."));
			}

			ACharacter* SpawnedCharacter = nullptr;

			//Spawn at TEAM_A PlayerStarts if the ChoosenTeam is TEAM_A
			if (PState->GetTeamNumber() == TEAM_A && TeamASpawns.Num() > 0)
			{
				uint32 ArrayIndex = FMath::RandRange(0, TeamASpawns.Max() - 1);

				if (!TeamASpawns.IsValidIndex(ArrayIndex))
				{
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Invalid array index in TeamASpawns"));
					return;
				}

				SpawnedCharacter = GetWorld()->SpawnActor<ACharacter>(PC->SelectedCharacter[PC->SelectedCharacterIndex], TeamASpawns[ArrayIndex]);
			}
			
			//Spawn at TEAM_B PlayerStarts if the ChoosenTeam is TEAM_B
			if (PState->GetTeamNumber() == TEAM_B && TeamBSpawns.Num() > 0)
			{
				uint32 ArrayIndex = FMath::RandRange(0, TeamBSpawns.Max() - 1);

				if (!TeamBSpawns.IsValidIndex(ArrayIndex))
				{
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Invalid array index in TeamBSpawns"));
					return;
				}

				SpawnedCharacter = GetWorld()->SpawnActor<ACharacter>(PC->SelectedCharacter[PC->SelectedCharacterIndex], TeamBSpawns[ArrayIndex]);
				
			}

			if (SpawnedCharacter)
			{
				//This cast does not require valid check because we already check if (SpawnedCharacter) is valid
				//ACharacter is a subclass of APawn so it's guaranteed to be valid
				APawn* SpawnedPawn = Cast<APawn>(SpawnedCharacter);

				//Posses the spawned Pawn
				PC->Possess(SpawnedPawn);
				
			}
			else
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("SpawnedCharacter is null."));
		}
		else
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("No Selected Character in MyPlayerController."));
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Invalid player controller or player state."));


}

bool ATeamDeathMatch::ChooseTeam(APlayerController* Player)
{
	int32 NumPlayersTeamA = 0;
	int32 NumPlayersTeamB = 0;
	int32 ChoosenTeam = TEAM_DEFAULT;

	//Loop through every player's PlayerState
	for (int32 i = 0; i < PlayerControllerArray.Num(); ++i)
	{

		ABasePlayerState* BasePState = Cast<ABasePlayerState>(PlayerControllerArray[i]->PlayerState);

		if (BasePState == nullptr)
			return false;

		//Count how many people are there in TEAM_A
		if (BasePState->GetTeamNumber() == TEAM_A)
			++NumPlayersTeamA;

		//Count how many people are there in TEAM_B
		if (BasePState->GetTeamNumber() == TEAM_B)
			++NumPlayersTeamB;

	}

	//PlayerState of the player that is currently joining this match
	ABasePlayerState* OurBasePS = Cast<ABasePlayerState>(Player->PlayerState);

	if (OurBasePS != nullptr)
	{
		//if both team have equal players then just choose a random team
		if (NumPlayersTeamA == NumPlayersTeamB)
			ChoosenTeam = FMath::RandRange(0, 1);

		//Self-explanatory
		if (NumPlayersTeamA > NumPlayersTeamB)
			ChoosenTeam = TEAM_B;
		else //if (NumPlayersTeamB > NumPlayersTeamA)
			ChoosenTeam = TEAM_A;

		//Set the TeamNumber for this player who is joining the match
		OurBasePS->SetTeamNumber(ChoosenTeam);

		return true;
	}
	

	return false;
}
