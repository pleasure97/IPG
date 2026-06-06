// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/GameFeatureAction_AddInputMappingContext.h"
#include "System/IPGAssetManager.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "GameFramework/PlayerController.h"

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureRegistering()
{
	Super::OnGameFeatureRegistering(); 

	RegisterInputMappingContext();
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureUnregistering()
{
	Super::OnGameFeatureUnregistering();

	UnregisterInputMappingContext();
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FInputMappingContextData& ActiveInputMappingContextData = InputMappingContextDataMap.FindOrAdd(Context); 

	if (!ensure(ActiveInputMappingContextData.ExtensionRequestHandles.IsEmpty()) ||
		!ensure(ActiveInputMappingContextData.ControllersAdded.IsEmpty()))
	{
		Reset(ActiveInputMappingContextData);
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddInputMappingContext::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FInputMappingContextData* ActiveInputMappingContextData = InputMappingContextDataMap.Find(Context); 
	if (ensure(ActiveInputMappingContextData))
	{
		Reset(*ActiveInputMappingContextData);
	}
}

#if WITH_EDITOR
#include "Misc/DataValidation.h"
EDataValidationResult UGameFeatureAction_AddInputMappingContext::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context), EDataValidationResult::Valid);

	int32 Index = 0;

	for (const FPriorityInputMappingContext& Entry : PriorityInputMappingContexts)
	{
		if (Entry.InputMappingContext.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::FromString("Null Input Mapping Context."));
		}
		++Index;
	}

	return Result;
}
#endif

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContext()
{
	RegisterInputMappingContextsHandle = FWorldDelegates::OnStartGameInstance.AddUObject(
		this, &UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextForGameInstance);
}

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextForGameInstance(UGameInstance* GameInstance)
{
	// Check if game instance is valid and local player added event is not bound to game feature yet
	if (IsValid(GameInstance) && !GameInstance->OnLocalPlayerAddedEvent.IsBoundToObject(this))
	{
		GameInstance->OnLocalPlayerAddedEvent.AddUObject(this, &UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextForLocalPlayer);
		GameInstance->OnLocalPlayerRemovedEvent.AddUObject(this, &UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContextForLocalPlayer);

		for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
		{
			RegisterInputMappingContextForLocalPlayer(*LocalPlayerIterator);
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::RegisterInputMappingContextForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	ManageInputMappingContextForLocalPlayer(LocalPlayer, /* bRegister */ true);
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContext()
{
	FWorldDelegates::OnStartGameInstance.Remove(RegisterInputMappingContextsHandle);
	RegisterInputMappingContextsHandle.Reset();

	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (TIndirectArray<FWorldContext>::TConstIterator WorldContextIterator = WorldContexts.CreateConstIterator(); WorldContextIterator; ++WorldContextIterator)
	{
		UnregisterInputMappingContextForGameInstance(WorldContextIterator->OwningGameInstance);
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContextForGameInstance(UGameInstance* GameInstance)
{
	if (IsValid(GameInstance))
	{
		GameInstance->OnLocalPlayerAddedEvent.RemoveAll(this);
		GameInstance->OnLocalPlayerRemovedEvent.RemoveAll(this);

		for (TArray<ULocalPlayer*>::TConstIterator LocalPlayerIterator = GameInstance->GetLocalPlayerIterator(); LocalPlayerIterator; ++LocalPlayerIterator)
		{
			UnregisterInputMappingContextForLocalPlayer(*LocalPlayerIterator);
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::UnregisterInputMappingContextForLocalPlayer(ULocalPlayer* LocalPlayer)
{
	ManageInputMappingContextForLocalPlayer(LocalPlayer, /* bRegister */ false);
}

void UGameFeatureAction_AddInputMappingContext::ManageInputMappingContextForLocalPlayer(ULocalPlayer* LocalPlayer, bool bRegister)
{
	ensure(LocalPlayer);

	UIPGAssetManager& IPGAssetManager = UIPGAssetManager::Get();

	if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
	{
		if (UEnhancedInputUserSettings* EnhancedInputUserSettings = EnhancedInputLocalPlayerSubsystem->GetUserSettings())
		{
			for (const FPriorityInputMappingContext& PriorityInputMappingContext : PriorityInputMappingContexts)
			{
				// Register this input mapping context with the settings
				if (UInputMappingContext* IMC = IPGAssetManager.GetAsset(PriorityInputMappingContext.InputMappingContext))
				{
					if (bRegister)
					{
						EnhancedInputUserSettings->RegisterInputMappingContext(IMC);
					}
					else
					{
						EnhancedInputUserSettings->UnregisterInputMappingContext(IMC);
					}
				}
			}
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::Reset(FInputMappingContextData& ActiveInputMappingContextData)
{
	ActiveInputMappingContextData.ExtensionRequestHandles.Empty(); 

	while (!ActiveInputMappingContextData.ControllersAdded.IsEmpty())
	{
		TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr = ActiveInputMappingContextData.ControllersAdded.Top();
		if (PlayerControllerWeakPtr.IsValid())
		{
			RemoveInputMappingContext(PlayerControllerWeakPtr.Get(), ActiveInputMappingContextData);
		}
		else
		{
			ActiveInputMappingContextData.ControllersAdded.Pop();
		}
	}
}

void UGameFeatureAction_AddInputMappingContext::RemoveInputMappingContext(APlayerController* PlayerController, FInputMappingContextData& ActiveInputMappingContextData)
{
	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			for (const FPriorityInputMappingContext& PriorityInputMappingContext : PriorityInputMappingContexts)
			{
				if (const UInputMappingContext* IMC = PriorityInputMappingContext.InputMappingContext.Get())
				{
					EnhancedInputLocalPlayerSubsystem->RemoveMappingContext(IMC);
				}
			}
		}
	}
}
