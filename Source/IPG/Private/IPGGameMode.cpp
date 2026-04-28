// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPGGameMode.h"
#include "Game/IPGGameState.h"
#include "Game/Experience/IPGExperienceManagerComponent.h"
#include "Player/IPGPlayerState.h"

AIPGGameMode::AIPGGameMode()
{

}

void AIPGGameMode::InitGameState()
{
	Super::InitGameState();

	// Listen for the experience load to complete
	UIPGExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UIPGExperienceManagerComponent>(); 
	check(ExperienceManagerComponent);

	ExperienceManagerComponent->CallOrRegisterExperienceLoadedCallback(
		FOnIPGExperienceLoaded::FDelegate::CreateUObject(this, &AIPGGameMode::OnExperienceLoaded)); 
}

const UIPGCharacterData* AIPGGameMode::GetCharacterDataForController(const AController* InController) const
{
	// See if character data is already set on the player state
	if (InController != nullptr)
	{
		if (const AIPGPlayerState* IPGPlayerState = InController->GetPlayerState<AIPGPlayerState>())
		{
			if (const UIPGCharacterData* CharacterData = IPGPlayerState->GetCharacterData())
			{
				return CharacterData;
			}
		}
	}
	return nullptr;
}

void AIPGGameMode::OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience)
{
	// TODO
}
