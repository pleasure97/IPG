// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/GameFeatureAction_AddInputBinding.h"
#include "Components/GameFrameworkComponentManager.h"

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
			UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate =
				UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
					this, &UGameFeatureAction_AddInputBinding::HandlePawnExtension, ChangeContext);
			TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle =
				ComponentManager->AddExtensionHandler(APawn::StaticClass(), AddAbilitiesDelegate);

			ActiveInputBindingContextData.ExtensionRequestHandles.Add(ExtensionRequestHandle);
		}
	}
}

void UGameFeatureAction_AddInputBinding::Reset(FInputBindingContextData& ActiveData)
{

}

void UGameFeatureAction_AddInputBinding::HandlePawnExtension(AActor* Actor, FName EventName, FGameFeatureStateChangeContext ChangeContext)
{
	APawn* ActorPawn = CastChecked<APawn>(Actor);
	FInputBindingContextData& ActiveData = InputBindingContextDataMap.FindOrAdd(ChangeContext);

	if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
	{
		RemoveInputMappingForPlayer(ActorPawn, ActiveData);
	}
	else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) /*|| (EventName == ULyraHeroComponent::NAME_BindInputsNow)*/)
	{
		AddInputMappingForPlayer(ActorPawn, ActiveData);
	}
}

void UGameFeatureAction_AddInputBinding::AddInputMappingForPlayer(APawn* Pawn, FInputBindingContextData& ActiveData)
{

}

void UGameFeatureAction_AddInputBinding::RemoveInputMappingForPlayer(APawn* Pawn, FInputBindingContextData& ActiveData)
{

}
