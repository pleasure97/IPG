// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IPGPlayerExtensionComponent.h"
#include "IPGGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"
#include "Ability/IPGAbilitySystemComponent.h"

const FName UIPGPlayerExtensionComponent::NAME_ActorFeatureName("PlayerExtension");

UIPGPlayerExtensionComponent::UIPGPlayerExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	CharacterData = nullptr;
	AbilitySystemComponent = nullptr;
}

FName UIPGPlayerExtensionComponent::GetFeatureName() const
{
	return NAME_ActorFeatureName;
}

bool UIPGPlayerExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();
	// To 'Spawned' Init State
	if (!CurrentState.IsValid() && DesiredState == IPGGameplayTags::InitState_Spawned)
	{
		// As long as we are on a valid pawn, we count as spawned
		if (Pawn)
		{
			return true;
		}
	}

	// 'Spawned' to 'Data Available' Init State
	if (CurrentState == IPGGameplayTags::InitState_Spawned && DesiredState == IPGGameplayTags::InitState_DataAvailable)
	{
		// Chracter data is required.
		if (!CharacterData)
		{
			return false;
		}

		const bool bHasAuthority = Pawn->HasAuthority();
		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Check for being possessed by a controller.
			if (!GetController<AController>())
			{
				return false;
			}
		}

		return true;
	}
	// 'Data Available' to 'Data Initialized' Init State
	else if (CurrentState == IPGGameplayTags::InitState_DataAvailable && DesiredState == IPGGameplayTags::InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		return Manager->HaveAllFeaturesReachedInitState(Pawn, IPGGameplayTags::InitState_DataAvailable);
	}
	// 'Data Initialized' to 'Gameplay Ready' Init State
	else if (CurrentState == IPGGameplayTags::InitState_DataInitialized && DesiredState == IPGGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UIPGPlayerExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// If another feature is now in 'Data Available' init state, see if we should transition to 'Data Initialized' init state
	if (Params.FeatureName != NAME_ActorFeatureName)
	{
		if (Params.FeatureState == IPGGameplayTags::InitState_DataAvailable)
		{
			CheckDefaultInitialization();
		}
	}
}

void UIPGPlayerExtensionComponent::CheckDefaultInitialization()
{
	// Before checking our progress, try progressing any other features we might depend on
	CheckDefaultInitializationForImplementers();

	static const TArray<FGameplayTag> StateChain = { IPGGameplayTags::InitState_Spawned, IPGGameplayTags::InitState_DataAvailable, IPGGameplayTags::InitState_DataInitialized, IPGGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay)
	// through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UIPGPlayerExtensionComponent::InitializeAbilitySystem(UIPGAbilitySystemComponent* InAbilitySystemComponent, AActor* InOwnerActor)
{
	check(InAbilitySystemComponent);
	check(InOwnerActor);

	// If ability system component has NOT changed
	if (AbilitySystemComponent == InAbilitySystemComponent)
	{
		return;
	}

	// Clean up the old ability system component
	if (AbilitySystemComponent)
	{
		UninitializeAbilitySystem();
	}
}

void UIPGPlayerExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ability system component if we're still the avatar actor 
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		// TODO - Add ability tag (e.g., Ability_Behaivor_SurvivesDeath)

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore); 
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
		else
		{
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}
	AbilitySystemComponent = nullptr;
}

void UIPGPlayerExtensionComponent::OnRegister()
{
	Super::OnRegister();

	// Ensure that pawn is not nullptr
	const APawn* Pawn = GetPawn<APawn>();
	ensureAlwaysMsgf((Pawn != nullptr), TEXT("IPGPlayerExtensionComponent on [%s] can only be added to Pawn actors."), *GetNameSafe(GetOwner()));

	// Ensure that pawn has only one player extension component
	TArray<UActorComponent*> PlayerExtensionComponents;
	Pawn->GetComponents(UIPGPlayerExtensionComponent::StaticClass(), PlayerExtensionComponents);
	ensureAlwaysMsgf((PlayerExtensionComponents.Num() == 1), TEXT("Only 1 IPGPlayerExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void UIPGPlayerExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for changes to all features
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Notifies state manager that we have spawned, then try rest of default initialization
	ensure(TryToChangeInitState(IPGGameplayTags::InitState_Spawned));

	CheckDefaultInitialization();
}

void UIPGPlayerExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UIPGPlayerExtensionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UIPGPlayerExtensionComponent, CharacterData);
}

void UIPGPlayerExtensionComponent::OnRep_CharacterData()
{
	CheckDefaultInitialization();
}
