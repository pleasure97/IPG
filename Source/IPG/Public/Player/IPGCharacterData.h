// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGCharacterData.generated.h"

class UIPGAbilitySet;
class UIPGInputConfig;

/**
 * 
 */
UCLASS(BlueprintType, Const)
class IPG_API UIPGCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Class to instantiate for this pawn
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IPG|Character")
	TSubclassOf<APawn> PawnClass;

	// Ability sets to grant to this pawn's ability system.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Abilities")
	TArray<TObjectPtr<UIPGAbilitySet>> AbilitySets;

	// Input configuration used by player controlled pawns to create input mappings and bind input actions.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lyra|Input")
	TObjectPtr<UIPGInputConfig> InputConfig;
};
