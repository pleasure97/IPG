// Fill out your copyright notice in the Description page of Project Settings.


#include "IPGGameplayTags.h"

namespace IPGGameplayTags
{
	/* Common UI - Widgets */
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_GuildScreen, "UI.Widget.GuildScreen");

	/* Common UI - Widget Stack */
	UE_DEFINE_GAMEPLAY_TAG(UI_WidgetStack_Modal, "UI.WidgetStack.Modal");
	UE_DEFINE_GAMEPLAY_TAG(UI_WidgetStack_GameMenu, "UI.WidgetStack.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_WidgetStack_GameHUD, "UI.WidgetStack.GameHUD");
	UE_DEFINE_GAMEPLAY_TAG(UI_WidgetStack_FrontEnd, "UI.WidgetStack.FrontEnd");

	/* Game Feature - Init State */
	UE_DEFINE_GAMEPLAY_TAG(InitState_Spawned, "InitState.Spawned");
	UE_DEFINE_GAMEPLAY_TAG(InitState_DataAvailable, "InitState.DataAvailable");
	UE_DEFINE_GAMEPLAY_TAG(InitState_DataInitialized, "InitState.DataInitialized");
	UE_DEFINE_GAMEPLAY_TAG(InitState_GameplayReady, "InitState.GameplayReady");

	/* Input Tag */
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Move, "InputTag.Move");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_Look, "InputTag.Look");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_ToggleInventory, "InputTag.ToggleInventory");
	UE_DEFINE_GAMEPLAY_TAG(InputTag_PickUp, "InputTag.PickUp");
}
