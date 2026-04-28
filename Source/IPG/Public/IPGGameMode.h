// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IPGGameMode.generated.h"

class UIPGExperienceDefinition; 
class UIPGCharacterData;

/**
 * 
 */
UCLASS(MinimalAPI, Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class AIPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AIPGGameMode();

	/* AGameModeBase Interface */
	virtual void InitGameState() override;

	const UIPGCharacterData* GetCharacterDataForController(const AController* InController) const;

private:
	void OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience);
};



