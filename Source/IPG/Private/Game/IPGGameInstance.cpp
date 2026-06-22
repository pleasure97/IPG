// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/IPGGameInstance.h"
#include "Components/GameFrameworkComponentManager.h"
#include "IPGGameplayTags.h"

UIPGGameInstance::UIPGGameInstance()
{
}

void UIPGGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* GameFrameworkComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(GameFrameworkComponentManager))
	{
		GameFrameworkComponentManager->RegisterInitState(IPGGameplayTags::InitState_Spawned, false, FGameplayTag());
		GameFrameworkComponentManager->RegisterInitState(IPGGameplayTags::InitState_DataAvailable, false, IPGGameplayTags::InitState_Spawned);
		GameFrameworkComponentManager->RegisterInitState(IPGGameplayTags::InitState_DataInitialized, false, IPGGameplayTags::InitState_DataAvailable);
		GameFrameworkComponentManager->RegisterInitState(IPGGameplayTags::InitState_GameplayReady, false, IPGGameplayTags::InitState_DataInitialized);
	}
}

