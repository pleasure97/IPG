// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"

namespace IPGGameplayTags
{
	/* Common UI - Widgets */
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_GuildScreen);

	/* Common UI - Widget Stack */
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_WidgetStack_Modal);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_WidgetStack_GameMenu);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_WidgetStack_GameHUD);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_WidgetStack_FrontEnd);

	/* Game Feature - Init State */
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_Spawned);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataAvailable);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_DataInitialized);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InitState_GameplayReady);

	/* Input Tag */
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_ToggleInventory);
	IPG_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_PickUp);
}