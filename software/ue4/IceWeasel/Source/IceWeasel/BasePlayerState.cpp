// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BasePlayerState.h"


ABasePlayerState::ABasePlayerState()
{
	TeamNum = -1;
}

void ABasePlayerState::SetTeamNumber(int TeamNumber)
{
	ServerSetTeamNumber(TeamNumber);
}


int ABasePlayerState::GetTeamNumber()const
{
	return TeamNum;
}

#pragma region Server RPCs 
void ABasePlayerState::ServerSetTeamNumber_Implementation(int TeamNumber)
{
	TeamNum = TeamNumber;
}

bool ABasePlayerState::ServerSetTeamNumber_Validate(int TeamNumber)
{
	return true;
}

#pragma endregion 


void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, TeamNum);

}