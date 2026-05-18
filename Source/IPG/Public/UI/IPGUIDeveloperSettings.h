// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "IPGUIDeveloperSettings.generated.h"

class UIPGActivatableWidget;

/**
 * 
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "IPG UI Settings"))
class IPG_API UIPGUIDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UIPGUIDeveloperSettings();

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Widget Reference", meta = (ForceInlineRow, Categories = "IPGUI.Widget"))
	TMap<FGameplayTag, TSoftClassPtr<UIPGActivatableWidget>> IPGUIWidgetMap;
};
