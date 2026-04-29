// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPGGameMode.h"
#include "Game/IPGGameState.h"
#include "Game/Experience/IPGExperienceManagerComponent.h"
#include "Game/Experience/IPGExperienceDefinition.h"
#include "Player/IPGPlayerState.h"
#include "Player/IPGCharacterData.h"
#include "System/IPGAssetManager.h"

AIPGGameMode::AIPGGameMode()
{
	GameStateClass = AIPGGameState::StaticClass();
	PlayerStateClass = AIPGPlayerState::StaticClass();
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

void AIPGGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage); 

	// Wait for the next frame to give time to initialize startup settings
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AIPGGameMode::AssignDefaultExperience);
}

void AIPGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Delay starting new players until the experience has been loaded
	// (players who login prior to that will be started by OnExperienceLoaded)
	if (IsExperienceLoaded())
	{
		Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	}
}

UClass* AIPGGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const UIPGCharacterData* CharacterData = GetCharacterDataForController(InController))
	{
		if (CharacterData->PawnClass)
		{
			return CharacterData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
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

	// If not, fall back to the the default for the current experience
	check(GameState);
	UIPGExperienceManagerComponent* ExperienceManagerComponent = GameState->FindComponentByClass<UIPGExperienceManagerComponent>();
	check(ExperienceManagerComponent);

	if (ExperienceManagerComponent->IsExperienceLoaded())
	{
		const UIPGExperienceDefinition* Experience = ExperienceManagerComponent->GetCurrentExperienceChecked();
		if (Experience->CharacterData != nullptr)
		{
			return Experience->CharacterData;
		}
	}

	return nullptr;
}

bool AIPGGameMode::IsExperienceLoaded() const
{
	check(GameState);
	UIPGExperienceManagerComponent* FoundExperienceComponent = GameState->FindComponentByClass<UIPGExperienceManagerComponent>();
	check(FoundExperienceComponent);

	return FoundExperienceComponent->IsExperienceLoaded();
}

void AIPGGameMode::AssignDefaultExperience()
{
	FPrimaryAssetId ExperienceId;
	FString ExperienceIdSource;

	// Final fallback to the default experience
	if (!ExperienceId.IsValid())
	{
		//@TODO: Pull this from a config setting or something
		ExperienceId = FPrimaryAssetId(FPrimaryAssetType("IPGExperienceDefinition"), FName("BP_IPGDefaultExperience"));
		ExperienceIdSource = TEXT("Default");
	}

	GrantExperience(ExperienceId, ExperienceIdSource);
}

void AIPGGameMode::GrantExperience(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource)
{
	if (!ExperienceId.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Experience ID is not valid in AIPGGameMode::GrantExperience()"));
		return;
	}

	UIPGExperienceManagerComponent* FoundExperienceComponent = GameState->FindComponentByClass<UIPGExperienceManagerComponent>();
	check(FoundExperienceComponent);
	FoundExperienceComponent->SetCurrentExperience(ExperienceId);
}

void AIPGGameMode::OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience)
{
	// Spawn any players that are already attached
	// GetDefaultPawnClassForController_Implementation might only be getting called for players anyways
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Cast<APlayerController>(*Iterator);
		if ((PC != nullptr) && (PC->GetPawn() == nullptr))
		{
			if (PlayerCanRestart(PC))
			{
				RestartPlayer(PC);
			}
		}
	}
}
