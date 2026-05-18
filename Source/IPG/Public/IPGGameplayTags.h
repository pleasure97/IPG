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
}