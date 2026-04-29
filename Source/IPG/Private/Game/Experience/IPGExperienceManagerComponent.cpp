// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Experience/IPGExperienceManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "System/IPGAssetManager.h"
#include "Game/Experience/IPGExperienceDefinition.h"
#include "Game/Experience/IPGExperienceActionSet.h"
#include "GameFeaturesSubsystemSettings.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatureAction.h"

UIPGExperienceManagerComponent::UIPGExperienceManagerComponent()
{
	SetIsReplicatedByDefault(true);
}

void UIPGExperienceManagerComponent::CallOrRegisterExperienceLoadedCallback(FOnIPGExperienceLoaded::FDelegate&& Delegate)
{
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		OnExperienceLoaded.Add(MoveTemp(Delegate)); 
	}
}

void UIPGExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	UIPGAssetManager& AssetManager = UIPGAssetManager::Get();
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(ExperienceId);
	TSubclassOf<UIPGExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad()); 
	check(AssetClass); 
	const UIPGExperienceDefinition* Experience = GetDefault<UIPGExperienceDefinition>(AssetClass); 

	check(Experience != nullptr); 
	check(CurrentExperience == nullptr); 

	CurrentExperience = Experience;

	StartExperienceLoad();
}

bool UIPGExperienceManagerComponent::IsExperienceLoaded() const
{
	return (LoadState == EIPGExperienceLoadState::Loaded) && (CurrentExperience != nullptr);
}

const UIPGExperienceDefinition* UIPGExperienceManagerComponent::GetCurrentExperienceChecked() const
{
	check(LoadState == EIPGExperienceLoadState::Loaded);
	check(CurrentExperience != nullptr);
	return CurrentExperience;
}

void UIPGExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentExperience);
}

void UIPGExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	// decrement the number of plugins that are loading
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullLoadCompleted();
	}
}

void UIPGExperienceManagerComponent::OnRep_CurrentExperience()
{
	StartExperienceLoad();
}

/**
 * Asynchronous loading process for the current experience.
 * 1. Validate the experience state.
 * 2. Gather Primary Asset IDs from the experience definition and associated action sets.
 * 3. Determine the appropriate asset bundles to load based on the network context (Client vs. Server).
 * 4. Coordinate multiple async load handles and binding completion delegates.
 */
void UIPGExperienceManagerComponent::StartExperienceLoad()
{
	// Ensure an experience is defined and we aren't already loading something
	check(CurrentExperience != nullptr);
	check(LoadState == EIPGExperienceLoadState::Unloaded);

	// Transition to loading state
	LoadState = EIPGExperienceLoadState::Loading;

	UIPGAssetManager& AssetManager = UIPGAssetManager::Get();

	TSet<FPrimaryAssetId> BundleAssetList;
	TSet<FSoftObjectPath> RawAssetList;

	// Collect the experience asset itself and all associated experience action sets
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());
	for (const TObjectPtr<UIPGExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	// Determine which asset bundles are required for the current execution context
	TArray<FName> BundlesToLoad;
	BundlesToLoad.Add(FIPGBundles::Equipped);

	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || (OwnerNetMode != NM_DedicatedServer);
	const bool bLoadServer = GIsEditor || (OwnerNetMode != NM_Client);

	if (bLoadClient)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	// Begin loading assets via the Asset Manager
	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(
			BundleAssetList.Array(), BundlesToLoad, {}, false, FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority);
	}

	// Handle any raw asset paths if necessary
	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		RawLoadHandle = AssetManager.LoadAssetList(
			RawAssetList.Array(), FStreamableDelegate(), FStreamableManager::AsyncLoadHighPriority, TEXT("StartExperienceLoad()"));
	}

	// Consolidate handles if multiple async operations are in flight
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({ BundleLoadHandle, RawLoadHandle });
	}
	else
	{
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	// Setup completion delegate
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(
		this, &UIPGExperienceManagerComponent::OnExperienceLoadComplete);
	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// If no assets were needed or loading finished instantly, proceed to completion
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		// Bind to completion and handle potential cancellation
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda([OnAssetsLoadedDelegate]()
			{
				OnAssetsLoadedDelegate.ExecuteIfBound();
			}));
	}

	// Optional: Fire-and-forget preloading for non-critical assets
	TSet<FPrimaryAssetId> PreloadAssetList;
	//@TODO: Determine assets to preload (but not blocking-ly)
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

/**
 * Callback triggered when the primary assets for the experience have finished loading.
 * 1. Identify and validate Game Feature Plugins required by the experience and its action sets.
 * 2. Initiate the asynchronous loading and activation of these plugins.
 * 3. Transition the load state to 'LoadingGameFeatures' or completing the process if no plugins are needed.
 */
void UIPGExperienceManagerComponent::OnExperienceLoadComplete()
{
	// Validate that we are in the expected state and have a valid experience definition
	check(LoadState == EIPGExperienceLoadState::Loading);
	check(CurrentExperience != nullptr);

	// Reset the list of plugin URLs to be loaded
	GameFeaturePluginURLs.Reset();

	// Helper lambda to resolve plugin names into functional URLs via the GameFeaturesSubsystem.
	// Filters out duplicates and warns if a specified plugin cannot be found
	auto CollectGameFeaturePluginURLs = [this](const UPrimaryDataAsset* Context, const TArray<FString>& FeaturePluginList)
		{
			for (const FString& PluginName : FeaturePluginList)
			{
				FString PluginURL;
				if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName, /*out*/ PluginURL))
				{
					GameFeaturePluginURLs.AddUnique(PluginURL);
				}
			}
		};

	// Gather plugin URLs from the main experience definition
	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);

	// Gather additional plugin URLs from all associated action sets
	for (const TObjectPtr<UIPGExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}

	// Start loading and activating the identified Game Feature Plugins
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();
	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = EIPGExperienceLoadState::LoadingGameFeatures;

		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			// Notify the global manager and request the subsystem to load/activate the plugin
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(
				PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(
					this, &UIPGExperienceManagerComponent::OnGameFeaturePluginLoadComplete));
		}
	}
	else
	{
		OnExperienceFullLoadCompleted();
	}
}

/**
 * Finalize the experience loading process.
 * 1. Execute Game Feature Actions (Registering, Loading, Activating).
 * 2. Broadcast completion delegates to other systems.
 */
void UIPGExperienceManagerComponent::OnExperienceFullLoadCompleted()
{
	check(LoadState != EIPGExperienceLoadState::Loaded);

	LoadState = EIPGExperienceLoadState::ExecutingActions;

	// Setup the activation context for Game Feature Actions
	FGameFeatureActivatingContext GameFeatureActivatingContext;

	// Ensure actions are restricted to the current world context
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	if (ExistingWorldContext)
	{
		GameFeatureActivatingContext.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	/** 
	 * Helper lambda to iterate through a list of actions and trigger their lifecycle events.
	 * Actions go through Registering -> Loading -> Activating.
	 */
	auto ActivateListOfActions = [&GameFeatureActivatingContext](const TArray<UGameFeatureAction*>& ActionList)
		{
			for (UGameFeatureAction* Action : ActionList)
			{
				if (Action != nullptr)
				{
					//@TODO: The fact that these don't take a world are potentially problematic in client-server PIE
					// The current behavior matches systems like gameplay tags where loading and registering apply to the entire process,
					// but actually applying the results to actors is restricted to a specific world
					Action->OnGameFeatureRegistering();
					Action->OnGameFeatureLoading();
					Action->OnGameFeatureActivating(GameFeatureActivatingContext);
				}
			}
		};

	// Execute actions from the primary experience definition
	ActivateListOfActions(CurrentExperience->Actions);

	// Execute actions from all included action sets
	for (const TObjectPtr<UIPGExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}

	// Final state transition: The experience is now fully loaded and active
	LoadState = EIPGExperienceLoadState::Loaded;

	// Broadcast completion to listeners
	OnExperienceLoaded.Broadcast(CurrentExperience); 
	OnExperienceLoaded.Clear();
}