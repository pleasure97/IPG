// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "IPGPlayerExtensionComponent.generated.h"

class UIPGCharacterData;
class UIPGAbilitySystemComponent;

/**
 * Component that adds functionality to all playable classes
 * This coordinates the initialization of other components.
 */
UCLASS()
class IPG_API UIPGPlayerExtensionComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
	
public:
	UIPGPlayerExtensionComponent(const FObjectInitializer& ObjectInitializer);

	// The name of this overall feature, this one depends on the other named component features */
	static const FName NAME_ActorFeatureName;

	const UIPGCharacterData* GetCharacterData() const { return CharacterData; }

	/* IGameFrameworkInitStateInterface interface */ 
	virtual FName GetFeatureName() const override;
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

	/* Ability System */
	// Should be called by the owning pawn to become the avatar of the ability system.
	void InitializeAbilitySystem(UIPGAbilitySystemComponent* InAbilitySystemComponent, AActor* InOwnerActor);
	// Should be called by the owning pawn to remove itself as the avatar of the ability system.
	void UninitializeAbilitySystem();
	// Gets the current ability system component, which may be owned by a different actor
	UFUNCTION(BlueprintPure, Category = "Lyra|Pawn")
	UIPGAbilitySystemComponent* GetIPGAbilitySystemComponent() const { return AbilitySystemComponent; }

protected:
	/* Actor Component override */
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Delegate fired when our pawn becomes the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	// Delegate fired when our pawn is removed as the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	// Character data used to create the pawn. Specified from a spawn function or on a placed instance. 
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_CharacterData)
	TObjectPtr<const UIPGCharacterData> CharacterData;

	// Pointer to the ability system component that is cached for convenience. 
	UPROPERTY(Transient)
	TObjectPtr<UIPGAbilitySystemComponent> AbilitySystemComponent;

	UFUNCTION()
	void OnRep_CharacterData();
};
