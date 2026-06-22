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
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	/* AGameModeBase Interface End */
	UFUNCTION(BlueprintCallable, Category = "IPG|Character")
	const UIPGCharacterData* GetCharacterDataForController(const AController* InController) const;

protected:
	bool IsExperienceLoaded() const;

	void AssignDefaultExperience(); 

	void GrantExperience(FPrimaryAssetId ExperienceId, const FString& ExperienceIdSource);

private:
	void OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience);
};



