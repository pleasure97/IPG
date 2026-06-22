// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "IPGPlayerState.generated.h"

class UIPGExperienceDefinition;
class UIPGCharacterData;
class UAbilitySystemComponent;
class UIPGAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class IPG_API AIPGPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AIPGPlayerState();

	/* AActor Interface */
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool IsSupportedForNetworking() const override { return true; }

	/* Character Data */
	const UIPGCharacterData* GetCharacterData() const { return CharacterData; }
	void SetCharacterData(const UIPGCharacterData* InCharacterData);

	/* Ability System Component */
	UIPGAbilitySystemComponent* GetIPGAbilitySystemComponent() const { return AbilitySystemComponent; }
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterData)
	TObjectPtr<const UIPGCharacterData> CharacterData;

	UFUNCTION()
	void OnRep_CharacterData();
	
private:
	void OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience);

	UPROPERTY()
	TObjectPtr<UIPGAbilitySystemComponent> AbilitySystemComponent;
};
