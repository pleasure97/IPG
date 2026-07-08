// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/GameFeatureAction_AddAbility.h"
#include "AbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Player/IPGPlayerState.h"

void UGameFeatureAction_AddAbility::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
	FPerContextData& ActiveData = ContextData.FindOrAdd(Context);

	if (!(ActiveData.ActiveExtensions.IsEmpty()) || !(ActiveData.ComponentRequests.IsEmpty()))
	{
		Reset(ActiveData); 
	}

	Super::OnGameFeatureActivating(Context);
}

void UGameFeatureAction_AddAbility::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context); 

	FPerContextData* ActiveData = ContextData.Find(Context); 

	if (ActiveData)
	{
		Reset(*ActiveData);
	}
}

void UGameFeatureAction_AddAbility::AddToWorld(const FWorldContext& WorldContext, const FGameFeatureStateChangeContext& ChangeContext)
{
	UWorld* World = WorldContext.World(); 
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;
	FPerContextData& ActiveData = ContextData.FindOrAdd(ChangeContext); 

	if (IsValid(GameInstance) && IsValid(World) && World->IsGameWorld())
	{
		if (UGameFrameworkComponentManager* GameFrameworkComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			int32 EntryIndex = 0; 
			for (const FGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilityDelegate =
						UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
							this, &UGameFeatureAction_AddAbility::HandleActorExtension, EntryIndex, ChangeContext);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle 
						= GameFrameworkComponentManager->AddExtensionHandler(Entry.ActorClass, AddAbilityDelegate);
					ActiveData.ComponentRequests.Add(ExtensionRequestHandle);
					++EntryIndex;
				}
			}
		}
	}
}

void UGameFeatureAction_AddAbility::HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex, FGameFeatureStateChangeContext ChangeContext)
{
	FPerContextData* ActiveData = ContextData.Find(ChangeContext);
	if (AbilitiesList.IsValidIndex(EntryIndex) && ActiveData)
	{
		const FGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved) || (EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved))
		{
			RemoveActorAbilities(Actor, *ActiveData);
		}
		else if ((EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded) || (EventName == AIPGPlayerState::NAME_AbilityReady))
		{
			AddActorAbilities(Actor, Entry, *ActiveData);
		}
	}
}

void UGameFeatureAction_AddAbility::Reset(FPerContextData& ActiveData)
{
	while (!ActiveData.ActiveExtensions.IsEmpty())
	{
		auto ExtenstionIt = ActiveData.ActiveExtensions.CreateIterator(); 
		RemoveActorAbilities(ExtenstionIt->Key, ActiveData);
	}
	ActiveData.ComponentRequests.Empty();
}

void UGameFeatureAction_AddAbility::AddActorAbilities(AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
{
	check(Actor); 

	if (!Actor->HasAuthority())
	{
		return;
	}

	// Early return if ability extensions has been applied
	if (ActiveData.ActiveExtensions.Find(Actor))
	{
		return;
	}

	if (UAbilitySystemComponent* ASC = FindOrAddComponentForActor<UAbilitySystemComponent>(Actor, AbilitiesEntry, ActiveData))
	{
		// Reserve Member Variables of FActorExtensions
		FActorExtensions AddedExtensions;
		AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num()); 
		AddedExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAttributes.Num());
		AddedExtensions.AbilitySetHandles.Reserve(AbilitiesEntry.GrantedAbilitySets.Num());

		// Granted Abilities of Abilities Entry
		for (const FIPGAbilityGrant& GrantAbility : AbilitiesEntry.GrantedAbilities)
		{
			if (!GrantAbility.AbilityType.IsNull())
			{
				FGameplayAbilitySpec NewAbilitySpec(GrantAbility.AbilityType.LoadSynchronous());
				FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(NewAbilitySpec);

				AddedExtensions.Abilities.Add(AbilityHandle);
			}
		}

		// Granted Abilities of Abilities Entry
		for (const FIPGAttributeSetGrant& GrantedAttributeSet : AbilitiesEntry.GrantedAttributes)
		{
			if (!GrantedAttributeSet.AttributeSetType.IsNull())
			{
				TSubclassOf<UAttributeSet> SetType = GrantedAttributeSet.AttributeSetType.LoadSynchronous(); 
				if (SetType)
				{
					UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetType);
					if (!GrantedAttributeSet.InitializationData.IsNull())
					{
						UDataTable* InitData = GrantedAttributeSet.InitializationData.LoadSynchronous(); 
						if (InitData)
						{
							NewAttributeSet->InitFromMetaDataTable(InitData);
						}
					}
					AddedExtensions.Attributes.Add(NewAttributeSet); 
					ASC->AddAttributeSetSubobject(NewAttributeSet);
				}
			}
		}

		for (const TSoftObjectPtr<const UIPGAbilitySet>& AbilitySetPtr : AbilitiesEntry.GrantedAbilitySets)
		{
			if (const UIPGAbilitySet* IPGAbilitySet = AbilitySetPtr.Get())
			{
				IPGAbilitySet->GiveToAbilitySystem(ASC, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef()); 
			}
		}
		ActiveData.ActiveExtensions.Add(Actor, AddedExtensions);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to find or add an ability component. Abilities will not be granted."));
	}
}

void UGameFeatureAction_AddAbility::RemoveActorAbilities(AActor* Actor, FPerContextData& ActiveData)
{
	if (FActorExtensions* ActorExtensions = ActiveData.ActiveExtensions.Find(Actor))
	{
		if (UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
		{
			for (UAttributeSet* AS : ActorExtensions->Attributes)
			{
				ASC->RemoveSpawnedAttribute(AS);
			}

			for (FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->Abilities)
			{
				ASC->SetRemoveAbilityOnEnd(AbilityHandle);
			}

			for (FIPGAbilitySet_GrantedHandles& SetHandle : ActorExtensions->AbilitySetHandles)
			{
				SetHandle.TakeFromAbilitySystem(ASC);
			}
		}

		ActiveData.ActiveExtensions.Remove(Actor);
	}
}

UActorComponent* UGameFeatureAction_AddAbility::FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FGameFeatureAbilitiesEntry& AbilitiesEntry, FPerContextData& ActiveData)
{
	UActorComponent* Component = Actor->FindComponentByClass(ComponentType); 
	bool bShouldMakeComponentRequest = (Component == nullptr); 
	if (Component)
	{
		// Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
		// `Native` is what `CreationMethod` defaults to for dynamically added components.
		if (Component->CreationMethod == EComponentCreationMethod::Native)
		{
			// Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
			// If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
			UObject* ComponentArchetype = Component->GetArchetype(); 
			bShouldMakeComponentRequest = Component->HasAnyFlags(RF_ClassDefaultObject); 
		}
	}

	if (bShouldMakeComponentRequest)
	{
		UWorld* World = Actor->GetWorld(); 
		UGameInstance* GameInstance = World->GetGameInstance(); 

		if (UGameFrameworkComponentManager* GameFrameworkComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			TSharedPtr<FComponentRequestHandle> ComponentRequestHandle = GameFrameworkComponentManager->AddComponentRequest(AbilitiesEntry.ActorClass, ComponentType);
			ActiveData.ComponentRequests.Add(ComponentRequestHandle);
		}

		if (!Component)
		{
			Component = Actor->FindComponentByClass(ComponentType);
		}
	}
	return Component;
}
