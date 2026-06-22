// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/GameFeatureAction_AddInputBinding.h"
#include "Components/GameFrameworkComponentManager.h"
#include "EnhancedInputSubsystems.h"
#include "Player/IPGCharacterComponent.h"
#include "Input/IPGInputConfig.h"

void UGameFeatureAction_AddInputBinding::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FInputBindingContextData& ActiveInputBindingContextData = InputBindingContextDataMap.FindOrAdd(Context);
	if (!ensure(ActiveInputBindingContextData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveInputBindingContextData.PawnsAddedTo.IsEmpty()))
	{
		Reset(ActiveInputBindingContextData);
	}
	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputBinding::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FInputBindingContextData* ActiveInputBindingContextData = InputBindingContextDataMap.Find(Context);

	if (ensure(ActiveInputBindingContextData))
	{
		Reset(*ActiveInputBindingContextData);
	}
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"
EDataValidationResult UGameFeatureAction_AddInputBinding::IsDataValid(FDataValidationContext& Context) const
{
    return EDataValidationResult();
}
#endif

void UGameFeatureAction_AddInputBinding::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FInputBindingContextData& ActiveInputBindingContextData = InputBindingContextDataMap.FindOrAdd(ChangeContext);

	if (IsValid(GameInstance) && IsValid(World) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			// Bind a extension handler delegate from game framework component manager
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
					this, &UGameFeatureAction_AddInputBinding::HandleControllerExtension, ChangeContext);

			// Add the extension handler to game framework component manager
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
				ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddAbilitiesDelegate);

			ActiveInputBindingContextData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddInputBinding::Reset(FInputBindingContextData& ActiveData)
{

}

void UGameFeatureAction_AddInputBinding::HandleControllerExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APlayerController* PlayerController = CastChecked<APlayerController>(Actor);
	APawn* PlayerPawn = PlayerController->GetPawn();
	if (!IsValid(PlayerPawn))
	{
		return;
	}

	FInputBindingContextData& ActiveData = InputBindingContextDataMap.FindOrAdd(ChangeContext);

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveInputMappingForPlayer(PlayerPawn, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == UIPGCharacterComponent::NAME_BindInputsNow))
	{
		AddInputMappingForPlayer(PlayerPawn, ActiveData);
	}
}

void UGameFeatureAction_AddInputBinding::AddInputMappingForPlayer(APawn* Pawn, FInputBindingContextData& ActiveData)
{
	// Get Player Controller from Pawn
	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

	// Get Enhanced Input Local Player Subsystem
	if (ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			UIPGCharacterComponent* IPGCharacterComponent = Pawn->FindComponentByClass<UIPGCharacterComponent>();
			if (IPGCharacterComponent && IPGCharacterComponent->IsReadyToBindInputs())
			{
				for (const TSoftObjectPtr<const UIPGInputConfig>& InputConfigEntry : InputConfigs)
				{
					if (const UIPGInputConfig* BindSet = InputConfigEntry.LoadSynchronous())
					{
						IPGCharacterComponent->AddAdditionalInputConfig(BindSet);
					}
				}
			}
			ActiveData.PawnsAddedTo.AddUnique(Pawn);
		}
	}
}

void UGameFeatureAction_AddInputBinding::RemoveInputMappingForPlayer(APawn* Pawn, FInputBindingContextData& ActiveData)
{
	// Get Player Controller from Pawn
	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());

	// Get Enhanced Input Local Player Subsystem
	if (ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr)
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (UIPGCharacterComponent* IPGCharacterComponent = Pawn->FindComponentByClass<UIPGCharacterComponent>())
			{
				for (const TSoftObjectPtr<const UIPGInputConfig>& InputConfigEntry : InputConfigs)
				{
					if (const UIPGInputConfig* InputConfig = InputConfigEntry.Get())
					{
						IPGCharacterComponent->RemoveAdditionalInputConfig(InputConfig);
					}
				}
			}
		}
	}

	ActiveData.PawnsAddedTo.Remove(Pawn);
}
