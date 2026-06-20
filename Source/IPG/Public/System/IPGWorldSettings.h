// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "IPGWorldSettings.generated.h"

class UIPGExperienceDefinition;
/**
 * The default world settings object, used primarily to set the default gameplay experience to use when playing on this map 
 */
UCLASS()
class IPG_API AIPGWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	AIPGWorldSettings(); 

	FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<UIPGExperienceDefinition> DefaultGameplayExperience;
};
