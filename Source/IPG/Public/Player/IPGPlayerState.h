// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "IPGPlayerState.generated.h"

class UIPGExperienceDefinition;
class UIPGCharacterData;

/**
 * 
 */
UCLASS()
class IPG_API AIPGPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	/* AActor Interface */
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/* Character Data */
	const UIPGCharacterData* GetCharacterData() const { return CharacterData; }
	void SetCharacterData(const UIPGCharacterData* InCharacterData);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterData)
	TObjectPtr<const UIPGCharacterData> CharacterData;

	UFUNCTION()
	void OnRep_CharacterData();
	
private:
	void OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience);
};
