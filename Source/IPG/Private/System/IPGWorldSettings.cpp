// Fill out your copyright notice in the Description page of Project Settings.


#include "System/IPGWorldSettings.h"
#include "Engine/AssetManager.h"

AIPGWorldSettings::AIPGWorldSettings()
{
}

FPrimaryAssetId AIPGWorldSettings::GetDefaultGameplayExperience() const
{
    FPrimaryAssetId Result;
    if (!DefaultGameplayExperience.IsNull())
    {
        Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath()); 
    }

    return Result;
}
