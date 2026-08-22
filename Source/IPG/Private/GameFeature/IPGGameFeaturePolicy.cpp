// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGGameFeaturePolicy.h"
#include "GameFeature/IPGPackageData.h"
#include "System/IPGPackageSettings.h"

bool UIPGGameFeaturePolicy::IsPluginAllowed(const FString& PluginURL, FString* OutReason) const
{
    // A virtual function called when the engine attempts to load a Game Feature plugin
    if (!Super::IsPluginAllowed(PluginURL, OutReason))
    {
        return false;
    }

#if WITH_EDITOR
    // Check if the plugin is included in AllowedPluginNames of the currently active package
    if (!bCacheValid)
    {
        UE_LOG(LogTemp, Log, TEXT("Cache is not valid in UIPGGameFeaturePolicy::IsPluginAllowed()"));
    }

    if (!bHasActivePackage)
    {
        return true;
    }

    const FString PluginName = FPaths::GetBaseFilename(PluginURL);

    if (AllowedPluginNames.Contains(PluginName))
    {
        return true;
    }

    // Loading is rejected, and OutReason is recorded
    if (OutReason)
    {
        *OutReason = FString::Printf(TEXT("Excluded by active package '%s'"), *ActivePackageDisplayName);
    }

    return false;
#else
    // Allowed in all runtime builds
    return true;
#endif // WITH_EDITOR
}

void UIPGGameFeaturePolicy::InitGameFeatureManager()
{
#if WITH_EDITOR
    // Register a delegate to receive a signal when the active package changes
    ActivePackageChangedHandle = UIPGPackageSettings::Get().OnActivePackageChanged.AddUObject(
        this, &UIPGGameFeaturePolicy::HandleChangedActivePackage);
#endif // WITH_EDITOR

    RebuildAllowedPluginCache();

    Super::InitGameFeatureManager(); 
}

void UIPGGameFeaturePolicy::ShutdownGameFeatureManager()
{
#if WITH_EDITOR
    if (ActivePackageChangedHandle.IsValid())
    {
        UIPGPackageSettings::Get(). OnActivePackageChanged.Remove(ActivePackageChangedHandle);
        ActivePackageChangedHandle.Reset();
    }
#endif // WITH_EDITOR
    AllowedPluginNames.Reset(); 
    bCacheValid = false;

    Super::ShutdownGameFeatureManager();
}

void UIPGGameFeaturePolicy::RebuildAllowedPluginCache()
{
    // Initialization step before caching
    AllowedPluginNames.Reset(); 
    bHasActivePackage = false; 
    ActivePackageDisplayName.Reset(); 

#if WITH_EDITOR
    // Retrieve the currently set active package data
    const UIPGPackageData* ActivePackage = UIPGPackageSettings::Get().GetDeterminedActivePackage(); 
    if (!IsValid(ActivePackage))
    {
        UE_LOG(LogTemp, Error, TEXT("There are no active packages. All game feature plugins are allowed."));
        return;
    }

    bHasActivePackage = true;
    ActivePackageDisplayName = ActivePackage->PackageName.IsEmpty() ? ActivePackage->GetName() : ActivePackage->PackageName;

    const TArray<FString> PluginNames = ActivePackage->GetAllPluginNames(); 
    AllowedPluginNames.Reserve(PluginNames.Num()); 

    // Itreate through the names of all plugins belonging to that package
    for (const FString& PluginName : PluginNames)
    {
        // Store the plugin name in AllowedPluginNames
        AllowedPluginNames.Add(PluginName);
    }

    UE_LOG(LogTemp, Log, TEXT("Active package '%s' - %d Allowed plugins"), *ActivePackageDisplayName, AllowedPluginNames.Num()); 
#endif // WITH_EDITOR
}

void UIPGGameFeaturePolicy::HandleChangedActivePackage(const UIPGPackageData* NewPackage)
{
    bCacheValid = false;
}
