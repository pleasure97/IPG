// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IPGCharacterComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "IPGGameplayTags.h"
#include "Player/IPGPlayerState.h"
#include "Player/IPGPlayerExtensionComponent.h"
#include "IPGPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Input/IPGInputConfig.h"
#include "Input/IPGInputComponent.h"
#include "Player/IPGCharacterData.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Ability/IPGAbilitySystemComponent.h"
#include "InputMappingContext.h"
#include "GameFeature/GameFeatureAction_AddInputMappingContext.h"
#include "Component/ItemTraceComponent.h"

const FName UIPGCharacterComponent::NAME_ActorFeatureName("IPGCharacter");
const FName UIPGCharacterComponent::NAME_BindInputsNow("BindInputsNow");

UIPGCharacterComponent::UIPGCharacterComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReadyToBindInputs = false;
}

bool UIPGCharacterComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

void UIPGCharacterComponent::AddAdditionalInputConfig(const UIPGInputConfig* InputConfig)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PlayerController = GetController<APlayerController>();
	check(PlayerController);

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	check(LocalPlayer);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UIPGPlayerExtensionComponent* PlayerExtensionComponent = Pawn->FindComponentByClass<UIPGPlayerExtensionComponent>())
	{
		UIPGInputComponent* IPGInputComponent = Pawn->FindComponentByClass<UIPGInputComponent>();
		if(ensure(IPGInputComponent))
		{
			TArray<uint32> BindHandles;

			// Bind ability actions
			IPGInputComponent->BindAbilityActions(
				InputConfig, 
				this, 
				&UIPGCharacterComponent::AbilityInputTagPressed, 
				&UIPGCharacterComponent::AbilityInputTagReleased, 
				/*out*/ BindHandles);
			
			// TODO - Bind native actions
			IPGInputComponent->BindNativeAction(
				InputConfig,
				IPGGameplayTags::InputTag_ToggleInventory,
				ETriggerEvent::Triggered,
				this,
				&UIPGCharacterComponent::Input_ToggleInventory);

			IPGInputComponent->BindNativeAction(
				InputConfig,
				IPGGameplayTags::InputTag_PickUp,
				ETriggerEvent::Triggered,
				this,
				&UIPGCharacterComponent::Input_PickUp);
		}
	}
}

void UIPGCharacterComponent::RemoveAdditionalInputConfig(const UIPGInputConfig* InputConfig)
{

}

FName UIPGCharacterComponent::GetFeatureName() const
{
	return NAME_ActorFeatureName;
}

bool UIPGCharacterComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	// To 'Spawned' Init State
	if (!CurrentState.IsValid() && DesiredState == IPGGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			return true;
		}
	}
	// 'Spawned' to 'Data Available' Init State
	else if (CurrentState == IPGGameplayTags::InitState_Spawned && DesiredState == IPGGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		/*if (!GetPlayerState<AIPGPlayerState>())
		{
			return false;
		}*/

		// If pawn is not simulated proxy, we need to wait for a controller with registered ownership of the player state.
		if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
		{
			AController* Controller = GetController<AController>();

			const bool bControllerPairedWithPlayerState = 
				(Controller != nullptr) && (Controller->PlayerState != nullptr) && (Controller->PlayerState->GetOwner() == Controller);

			if (!bControllerPairedWithPlayerState)
			{
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		// If pawn is an autonomous proxy
		if (bIsLocallyControlled && !bIsBot)
		{
			AIPGPlayerController* IPGPlayerController = GetController<AIPGPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !IPGPlayerController || !IPGPlayerController->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	// 'Data Available' to 'Data Initialized' Init State
	else if (CurrentState == IPGGameplayTags::InitState_DataAvailable && DesiredState == IPGGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		AIPGPlayerState* IPGPlayerState = GetPlayerState<AIPGPlayerState>();

		return IPGPlayerState && Manager->HasFeatureReachedInitState(
			Pawn, UIPGPlayerExtensionComponent::NAME_ActorFeatureName, IPGGameplayTags::InitState_DataInitialized);
	}
	// 'Data Initialized' to 'Gameplay Ready' Init State
	else if (CurrentState == IPGGameplayTags::InitState_DataInitialized && DesiredState == IPGGameplayTags::InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UIPGCharacterComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// 'Data Available' to 'Data Initialized' Init State
	if (CurrentState == IPGGameplayTags::InitState_DataAvailable && DesiredState == IPGGameplayTags::InitState_DataInitialized)
	{
		// Ensure that pawn and player state are valid
		APawn* Pawn = GetPawn<APawn>();
		AIPGPlayerState* IPGPlayerState = GetPlayerState<AIPGPlayerState>();
		if (!ensure(Pawn && IPGPlayerState))
		{
			return;
		}

		const UIPGCharacterData* IPGCharacterData = nullptr;

		// Initialize Ability System Component
		if (UIPGPlayerExtensionComponent* PlayerExtensionComponent = Pawn->FindComponentByClass<UIPGPlayerExtensionComponent>())
		{
			IPGCharacterData = PlayerExtensionComponent->GetCharacterData();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PlayerExtensionComponent->InitializeAbilitySystem(IPGPlayerState->GetIPGAbilitySystemComponent(), IPGPlayerState);
		}

		// Initialize Player Input
		if (AIPGPlayerController* IPGPlayerController = GetController<AIPGPlayerController>())
		{
			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
		}
	}
}

void UIPGCharacterComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	// 'PlayerExtension' Feature Name and 'Data Initialized' Init State 
	if (Params.FeatureName == UIPGPlayerExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == IPGGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

void UIPGCharacterComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { IPGGameplayTags::InitState_Spawned, IPGGameplayTags::InitState_DataAvailable, IPGGameplayTags::InitState_DataInitialized, IPGGameplayTags::InitState_GameplayReady };

	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UIPGCharacterComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogTemp, Error, TEXT("[UIPGCharacterComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));
	}
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

void UIPGCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UIPGPlayerExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(IPGGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UIPGCharacterComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UIPGCharacterComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent); 

	// Check if pawn is valid
	const APawn* Pawn = GetPawn<APawn>(); 
	if (!IsValid(Pawn))
	{
		return;
	}

	// Check if player controller is valid
	const APlayerController* PlayerController = GetController<APlayerController>();
	check(PlayerController);

	// Check if local player is valid
	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	check(LocalPlayer);

	// Get enhanced input local player subsystem from player controller
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(EnhancedInputLocalPlayerSubsystem);

	// EnhancedInputLocalPlayerSubsystem->ClearAllMappings();


	// Get default input mappings from character data's input config
	if (UIPGPlayerExtensionComponent* PlayerExtensionComponent = Pawn->FindComponentByClass<UIPGPlayerExtensionComponent>())
	{
		if (const UIPGCharacterData* IPGCharacterData = PlayerExtensionComponent->GetCharacterData())
		{
			if (const UIPGInputConfig* InputConfig = IPGCharacterData->InputConfig)
			{
				for (const FPriorityInputMappingContext& DefaultInputMapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = DefaultInputMapping.InputMappingContext.LoadSynchronous())
					{
						if (DefaultInputMapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = EnhancedInputLocalPlayerSubsystem->GetUserSettings())
							{
								Settings->RegisterInputMappingContext(IMC);
							}

							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							EnhancedInputLocalPlayerSubsystem->AddMappingContext(IMC, DefaultInputMapping.Priority, Options);
						}
					}
				}

				// IPG Input Component has some additional functions to map Gameplay Tags to an Input Action.
				UIPGInputComponent* IPGInputComponent = Cast<UIPGInputComponent>(PlayerInputComponent);
				if (ensure(IPGInputComponent))
				{
					// Add the key mappings that may have been set by the player
					IPGInputComponent->AddInputMappings(InputConfig, EnhancedInputLocalPlayerSubsystem); 

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					IPGInputComponent->BindAbilityActions(
						InputConfig, 
						this, 
						&UIPGCharacterComponent::AbilityInputTagPressed, 
						&UIPGCharacterComponent::AbilityInputTagReleased,
						/*out*/ BindHandles);
				}
			}
		}
	}

	if (!bReadyToBindInputs)
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PlayerController), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UIPGCharacterComponent::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UIPGPlayerExtensionComponent* PlayerExtensionComponent = Pawn->FindComponentByClass<UIPGPlayerExtensionComponent>())
		{
			if (UIPGAbilitySystemComponent* IPGAbilitySystemComponent = PlayerExtensionComponent->GetIPGAbilitySystemComponent())
			{
				IPGAbilitySystemComponent->AbilityInputTagPressed(InputTag);
			}
		}
	}
}

void UIPGCharacterComponent::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UIPGPlayerExtensionComponent* PlayerExtensionComponent = Pawn->FindComponentByClass<UIPGPlayerExtensionComponent>())
		{
			if (UIPGAbilitySystemComponent* IPGAbilitySystemComponent = PlayerExtensionComponent->GetIPGAbilitySystemComponent())
			{
				IPGAbilitySystemComponent->AbilityInputTagReleased(InputTag);
			}
		}
	}
}

void UIPGCharacterComponent::Input_ToggleInventory(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Pawn))
	{
		return;
	}

	APlayerController* PC = Pawn->GetController<APlayerController>();
	if (!IsValid(PC))
	{
		return;
	}

	UItemTraceComponent* ItemTraceComponent = PC->FindComponentByClass<UItemTraceComponent>();
	if (ItemTraceComponent)
	{
		ItemTraceComponent->ToggleInventory();
	}
}

void UIPGCharacterComponent::Input_PickUp(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!IsValid(Pawn))
	{
		return;
	}

	APlayerController* PC = Pawn->GetController<APlayerController>();
	if (!IsValid(PC))
	{
		return;
	}

	UItemTraceComponent* ItemTraceComponent = PC->FindComponentByClass<UItemTraceComponent>();
	if (ItemTraceComponent)
	{
		ItemTraceComponent->PrimaryInteract();
	}
}
