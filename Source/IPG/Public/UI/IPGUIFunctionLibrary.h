// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "IPGUIFunctionLibrary.generated.h"

class UIPGActivatableWidget;

/**
 * 
 */
UCLASS()
class IPG_API UIPGUIFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "IPG UI Function Library")
	static TSoftClassPtr<UIPGActivatableWidget> GetIPGSoftWidgetClassByTag(
		UPARAM(meta = (Categories = "UI.Widget"))FGameplayTag InWidgetTag);
};
