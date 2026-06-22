// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "IPGCharacterComponent.generated.h"

struct FPriorityInputMappingContext;
struct FGameplayTag;
struct FInputActionValue;
class UIPGInputConfig;

/**
 * Component that sets up input and camera handling for player controlled characters
 * This depends on a PlayerExtensionComponent to coordinate initialization.
 */
UCLASS(Blueprintable, Meta = (BlueprintSpawnableComponent))
class IPG_API UIPGCharacterComponent : public UPawnComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()
	
public:
	UIPGCharacterComponent(const FObjectInitializer& ObjectInitializer);

	// The name of this component-implemented feature
	static const FName NAME_ActorFeatureName;

	// The name of the extension event sent via UGameFrameworkComponentManager when ability inputs are ready to bind */
	static const FName NAME_BindInputsNow;

	// True if this is controlled by a real player and has progressed far enough in initialization where additional input bindings can be added
	bool IsReadyToBindInputs() const;

	// Adds mode-specific input config
	void AddAdditionalInputConfig(const UIPGInputConfig* InputConfig);

	// Removes a mode-specific input config if it has been added
	void RemoveAdditionalInputConfig(const UIPGInputConfig* InputConfig);

	/* IGameFrameworkInitStateInterface interface */
	virtual FName GetFeatureName() const override;
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;

protected:
	/* Actor Component */
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* Input */
	virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void Input_ToggleInventory(const FInputActionValue& InputActionValue);
	void Input_PickUp(const FInputActionValue& InputActionValue);

	UPROPERTY(EditAnywhere)
	TArray<FPriorityInputMappingContext> DefaultInputMappings;

	/** True when player input bindings have been applied, will never be true for non - players */
	bool bReadyToBindInputs;
};
