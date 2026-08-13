// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGBundleData.generated.h"

/**
 * 
 */
UCLASS()
class IPG_API UIPGBundleData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/* Build */
	UPROPERTY(EditAnywhere, Category = "Build")
	FString BundleName;
	
	UPROPERTY(EditAnywhere, Category = "Build", meta = (MultiLine = "true"))
	FString BundleDescription;

	/* Plugin */
	UPROPERTY(EditAnywhere, Category = "Plugins")
	TArray<FString> BundlePluginNames;

	UPROPERTY(EditAnywhere, Category = "Plugins")
	FString PluginNameFilter;

	TArray<FString> GetPluginNames() const; 

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

private:
	static TArray<FString> GetAllInstalledGameFeaturePluginNames();
};
