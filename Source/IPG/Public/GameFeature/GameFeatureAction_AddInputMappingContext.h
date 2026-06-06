// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeature/GameFeatureAction_WorldActionBase.h"
#include "GameFeatureAction_AddInputMappingContext.generated.h"

class UInputMappingContext; 
class UIPGInputConfig;
class APlayerController;
struct FComponentRequestHandle;

USTRUCT()
struct FPriorityInputMappingContext
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Input", meta=(AssetBundles="Client,Server"))
	TSoftObjectPtr<UInputMappingContext> InputMappingContext;

	// Higher priority inupt mapping will be prioritized over mappings with a lower priority 
	UPROPERTY(EditAnywhere, Category = "Input")
	int32 Priority = 0; 
};

/**
 * Adds InputMappingContext to local players' EnhancedInput system.
 * Expects that local players are set up to use the EnhancedInput system.
 */
UCLASS(meta = (DisplayName = "Add Input Mapping Context"))
class IPG_API UGameFeatureAction_AddInputMappingContext : public UGameFeatureAction_WorldActionBase
{
	GENERATED_BODY()

public:
	/* UGameFeatureAction Interface */
	virtual void OnGameFeatureRegistering() override;
	virtual void OnGameFeatureUnregistering() override;
	virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
	/* UGameFeatureAction Interface Ends */

	/* UObject Interface */
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
	/* UObject Interface Ends */

	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<FPriorityInputMappingContext> PriorityInputMappingContexts; 

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UIPGInputConfig> InputConfig;

private:
	struct FInputMappingContextData
	{
		TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
		TArray<TWeakObjectPtr<APlayerController>> ControllersAdded;
	};
	
	TMap<FGameFeatureStateChangeContext, FInputMappingContextData> InputMappingContextDataMap;

	// Delegate for when game instance is changed to register input mapping context
	FDelegateHandle RegisterInputMappingContextsHandle;

	/* Input Mapping Context Registration Util */
	void RegisterInputMappingContext();
	void RegisterInputMappingContextForGameInstance(UGameInstance* GameInstance);
	void RegisterInputMappingContextForLocalPlayer(ULocalPlayer* LocalPlayer);

	/* Input Mapping Context Unregistration Util */
	void UnregisterInputMappingContext();
	void UnregisterInputMappingContextForGameInstance(UGameInstance* GameInstance);
	void UnregisterInputMappingContextForLocalPlayer(ULocalPlayer* LocalPlayer);
	void Reset(FInputMappingContextData& ActiveInputMappingContextData);
	void RemoveInputMappingContext(APlayerController* PlayerController, FInputMappingContextData& ActiveInputMappingContextData);

	void ManageInputMappingContextForLocalPlayer(ULocalPlayer* LocalPlayer, bool bRegister);
};
