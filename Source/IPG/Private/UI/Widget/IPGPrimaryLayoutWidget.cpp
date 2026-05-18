// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/IPGPrimaryLayoutWidget.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "IPGGameplayTags.h"

UCommonActivatableWidgetContainerBase* UIPGPrimaryLayoutWidget::FindWidgetStackByTag(const FGameplayTag& InTag) const
{
	return RegisterWidgetStackMap.FindRef(InTag);
}

void UIPGPrimaryLayoutWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	RegisterWidgetStack(IPGGameplayTags::UI_WidgetStack_FrontEnd, WidgetStack_Frontend);
	RegisterWidgetStack(IPGGameplayTags::UI_WidgetStack_GameHUD, WidgetStack_GameHUD);
	RegisterWidgetStack(IPGGameplayTags::UI_WidgetStack_GameMenu, WidgetStack_GameMenu);
	RegisterWidgetStack(IPGGameplayTags::UI_WidgetStack_Modal, WidgetStack_Modal);
}

void UIPGPrimaryLayoutWidget::RegisterWidgetStack(UPARAM(meta = (Categories = "UI.WidgetStack")) FGameplayTag InStackTag, UCommonActivatableWidgetContainerBase* InStack)
{
	if (!IsDesignTime())
	{
		if (!RegisterWidgetStackMap.Contains(InStackTag))
		{
			RegisterWidgetStackMap.Add(InStackTag, InStack);
		}
	}
}
