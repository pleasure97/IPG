// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/IPGPlayerState.h"
#include "IPGGameMode.h"
#include "Game/Experience/IPGExperienceManagerComponent.h"
#include "Player/IPGCharacterData.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

void AIPGPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents(); 

	UWorld* World = GetWorld(); 
	if (World && World->GetNetMode() != NM_Client)
	{
		AGameStateBase* FoundGameState = World->GetGameState(); 
		check(FoundGameState);
		UIPGExperienceManagerComponent* FoundExperienceComponent = FoundGameState->FindComponentByClass<UIPGExperienceManagerComponent>();
		check(FoundExperienceComponent);
		FoundExperienceComponent->CallOrRegisterExperienceLoadedCallback(
			FOnIPGExperienceLoaded::FDelegate::CreateUObject(this, &AIPGPlayerState::OnExperienceLoaded));
	}
}

void AIPGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); 

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(AIPGPlayerState, CharacterData, SharedParams);
}

void AIPGPlayerState::SetCharacterData(const UIPGCharacterData* InCharacterData)
{
	check(InCharacterData);

	// Should be ROLE_Authority 
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	// Early return when character data already exists
	if (CharacterData)
	{
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(AIPGPlayerState, CharacterData, this);
	CharacterData = InCharacterData;

	ForceNetUpdate();
}

void AIPGPlayerState::OnRep_CharacterData()
{

}

void AIPGPlayerState::OnExperienceLoaded(const UIPGExperienceDefinition* CurrentExperience)
{
	if (AIPGGameMode* IPGGameMode = GetWorld()->GetAuthGameMode<AIPGGameMode>())
	{
		if (const UIPGCharacterData* NewCharacterData = IPGGameMode->GetCharacterDataForController(GetOwningController()))
		{
			SetCharacterData(NewCharacterData);
		}
	}
}
