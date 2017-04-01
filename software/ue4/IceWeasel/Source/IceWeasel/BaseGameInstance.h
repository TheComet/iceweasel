// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

/**
 * GameInstance persists through out the lifetime of the game
 */

UENUM(BlueprintType)
enum class EFindSessionResult : uint8
{
	Default UMETA(DisplayName = "Default"), //not searched for any sessions yet
	JoinableSession UMETA(DisplayName = "Joinable Session"), //found a session that can be joined
	NonJoinableSession UMETA(DisplayName = "NonJoinable Session"), //found a session but cannot be joined due to reasons like the server being full 
	FindSessionFailed UMETA(DisplayName = "FindSession Failed"), //FindSession function failed - make sure the OnlineSubSystem is not NULL or make sure the session is destroyed after leaving the match
	NoSessionsFound UMETA(DisplayName = "No Sessions Found") //Successfully searched for sessions but no sessions were found
};


UCLASS()
class ICEWEASEL_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UBaseGameInstance();

protected:
	UPROPERTY(BlueprintReadWrite)
	bool bIsLAN;
	
	UPROPERTY(BlueprintReadWrite)
	int MaxPlayers;

	UPROPERTY(BlueprintReadWrite)
	int MaxSessionResults;

	UPROPERTY(BlueprintReadOnly)
	EFindSessionResult FindSessionResult;

};
