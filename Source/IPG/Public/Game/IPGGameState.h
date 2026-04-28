// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "IPGGameState.generated.h"

class UIPGExperienceManagerComponent;

/**
 * 
 */
UCLASS()
class IPG_API AIPGGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AIPGGameState(); 

private:
	UPROPERTY()
	TObjectPtr<UIPGExperienceManagerComponent> ExperienceManagerComponent;
};
