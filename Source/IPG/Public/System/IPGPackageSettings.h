// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameFeature/IPGPackageData.h"
#include "IPGPackageSettings.generated.h"

/**
 * 
 */
UCLASS(config = EditorPerProjectUserSettings, DefaultConfig, meta = (DisplayName = "IPG Package"))
class UIPGPackageSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, config, Category = "Patch")
	TSoftObjectPtr<UIPGPackageData> ActivePackage;
};
