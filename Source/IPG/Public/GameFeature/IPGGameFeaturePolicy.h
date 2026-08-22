// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeaturesProjectPolicies.h"
#include "IPGGameFeaturePolicy.generated.h"

class UIPGPackageData;

/**
 * 
 */
UCLASS()
class IPG_API UIPGGameFeaturePolicy : public UDefaultGameFeaturesProjectPolicies
{
	GENERATED_BODY()
	
public:
	virtual bool IsPluginAllowed(const FString& PluginURL, FString* OutReason) const override;

	virtual void InitGameFeatureManager() override;
	virtual void ShutdownGameFeatureManager() override; 

	void RebuildAllowedPluginCache();

private:
	void HandleChangedActivePackage(const UIPGPackageData* NewPackage);

	TSet<FString> AllowedPluginNames;
	bool bCacheValid = false;
	bool bHasActivePackage = false;
	FString ActivePackageDisplayName; 

	FDelegateHandle ActivePackageChangedHandle;
};
