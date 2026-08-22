// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/AssetManagerTypes.h"
#include "IPGGameFeatureTypeData.generated.h"

class UTexture2D;

/**
 * A base for implementing creation workflows and validation for each type using Blueprints.
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class IPG_API UIPGGameFeaturePluginPolicy : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Feature Type")
	bool ValidatePlugin(const FString& PluginName, TArray<FText>& OutErrors) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Game Feature Type")
	void OnPluginCreated(const FString& PluginName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Game Feature Type")
	void OnPluginRenamed(const FString& OldName, const FString& NewName);
};

/**
 * Definition of content types by game (Arena, Emote, Weapon, Map...)
 */
UCLASS()
class IPG_API UIPGGameFeatureTypeData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Game Feature")
	FString TypeName;

	UPROPERTY(EditAnywhere, Category = "Game Feature")
	TSoftObjectPtr<UTexture2D> CustomThumbnailImage;

	UPROPERTY(EditAnywhere, Category = "Game Feature", meta = (MultiLine = "true"))
	FString Description;

	UPROPERTY(EditAnywhere, Category = "Game Feature")
	bool bShowInWizard = true;

	UPROPERTY(EditAnywhere, Category = "Game Feature")
	TSoftClassPtr<UIPGGameFeaturePluginPolicy> PluginPolicyClass;
	
	UPROPERTY(EditAnywhere, Category = "Asset Manager", meta = (TitleProperty = "PrimaryAssetType"))
	TArray<FPrimaryAssetTypeInfo> PrimaryAssetTypesToScan;
	
	FString GetDisplayName() const;

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
