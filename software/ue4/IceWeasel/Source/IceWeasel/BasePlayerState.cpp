// Fill out your copyright notice in the Description page of Project Settings.

#include "IceWeasel.h"
#include "BasePlayerState.h"


ABasePlayerState::ABasePlayerState()
{
	TeamId = -1;
}

void ABasePlayerState::SetTeamId(int Id)
{
	TeamId = Id;

	if (!HasAuthority())
		ServerSetTeamId(Id);
}


int ABasePlayerState::GetTeamId()const
{
	return TeamId;
}

#pragma region Server RPCs 
void ABasePlayerState::ServerSetTeamId_Implementation(int id)
{
	TeamId = id;
}

bool ABasePlayerState::ServerSetTeamId_Validate(int id)
{
	return true;
}

#pragma endregion 


void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABasePlayerState, TeamId, COND_SkipOwner);

}