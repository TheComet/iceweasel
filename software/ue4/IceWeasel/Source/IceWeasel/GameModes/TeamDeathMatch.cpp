// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "TeamDeathMatch.h"
#include "BasePlayerState.h"
#include "BasePlayerController.h"
#include "TeamPlayerStart.h"

void ATeamDeathMatch::BeginPlay()
{
	Super::BeginPlay();


}

void ATeamDeathMatch::PostLogin(APlayerController* NewPlayer)
{

	if (!ChooseTeam(NewPlayer))
	{
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("ChooseTeam() failed!"));
		return;
	}


	for (TActorIterator<ATeamPlayerStart> PlayerStartItr(GetWorld()); PlayerStartItr; ++PlayerStartItr)
	{
		ATeamPlayerStart* TPS = *PlayerStartItr;

		PlayerStarts.AddUnique(TPS);
	}

	TArray<FTransform> TeamASpawns;
	TArray<FTransform> TeamBSpawns;

	if (PlayerStarts.Num() > 0)
	{
		TeamASpawns.Reset();
		TeamBSpawns.Reset();

		for (int32 i = 0; i < PlayerStarts.Num(); ++i)
		{
			ATeamPlayerStart* TeamPlayerStart = Cast<ATeamPlayerStart>(PlayerStarts[i]);



			if (TeamPlayerStart->TeamNumber == TEAM_A)
				TeamASpawns.Add(TeamPlayerStart->GetTransform());

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

	if (PC && PState)
	{
		if (PC->SelectedCharacter)
		{
			if (PC->GetPawn())
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("UnPossed Existing pawn."));
				PC->UnPossess();
			}

			decltype(PC->SelectedCharacter->GetArchetype()) SpawnedCharacter = nullptr;

			if (!GetWorld())
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("No World."));


	
			if (PState->GetTeamNumber() == TEAM_A && TeamASpawns.Num() > 0)
			{
				const FTransform RandomSpawnPoint = TeamASpawns[FMath::RandRange(0, TeamASpawns.Max())];
				

				SpawnedCharacter = GetWorld()->SpawnActor<ACharacter>(PC->SelectedCharacter, RandomSpawnPoint);

				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("tried Spawing"));
			}
			
			if (PState->GetTeamNumber() == TEAM_B && TeamBSpawns.Num() > 0)
			{
				const FTransform RandomSpawnPoint = TeamBSpawns[FMath::RandRange(0, TeamBSpawns.Max())];

				SpawnedCharacter = GetWorld()->SpawnActor<ACharacter>(PC->SelectedCharacter, RandomSpawnPoint);
				
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("tried Spawing"));
			}

			if (SpawnedCharacter)
			{
				APawn* p = Cast<APawn>(SpawnedCharacter);

				if (p)
				{
					GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("woah!"));
					PC->Possess(p);
				}
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Possed character!"));
			}
			else
				GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("No Spawned character"));
		}
		else
			GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("No Selected Character in MyPlayerController."));
	}
	else
		GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("Invalid player controller or player state."));


	Super::PostLogin(NewPlayer);

}

/* Super basic team selection */
bool ATeamDeathMatch::ChooseTeam(APlayerController* Player)
{
	int32 NumPlayersTeamA = 0;
	int32 NumPlayersTeamB = 0;
	int32 ChoosenTeam = TEAM_DEFAULT;

	for (int32 i = 0; i < PlayerControllerArray.Num(); ++i)
	{

		ABasePlayerState* BasePState = Cast<ABasePlayerState>(PlayerControllerArray[i]->PlayerState);

		if (BasePState == nullptr)
			return false;

		if (BasePState->GetTeamNumber() == TEAM_A)
			++NumPlayersTeamA;

		if (BasePState->GetTeamNumber() == TEAM_B)
			++NumPlayersTeamB;

	}

	ABasePlayerState* BasePS = Cast<ABasePlayerState>(Player->PlayerState);

	if (BasePS != nullptr)
	{
		//if both team have equal players then just choose a random team
		if (NumPlayersTeamA == NumPlayersTeamB)
			ChoosenTeam = FMath::RandRange(TEAM_A, TEAM_B);

		//Self-explanatory
		if (NumPlayersTeamA > NumPlayersTeamB)
			ChoosenTeam = TEAM_B;
		else
			ChoosenTeam = TEAM_A;

		BasePS->SetTeamNumber(ChoosenTeam);

		return true;
	}
	

	return false;
}
