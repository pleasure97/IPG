// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/IPGUIFunctionLibrary.h"
#include "UI/IPGUIDeveloperSettings.h"

TSoftClassPtr<UIPGActivatableWidget> UIPGUIFunctionLibrary::GetIPGSoftWidgetClassByTag(UPARAM(meta = (Categories = "UI.Widget")) FGameplayTag InWidgetTag)
{
	const UIPGUIDeveloperSettings* IPGUIDeveloperSettings = GetDefault<UIPGUIDeveloperSettings>();

	checkf(IPGUIDeveloperSettings->IPGUIWidgetMap.Contains(InWidgetTag), TEXT("Could Not Find Corresponding Widget Under Tag %s"), *InWidgetTag.ToString());

	return IPGUIDeveloperSettings->IPGUIWidgetMap.FindRef(InWidgetTag);
}
