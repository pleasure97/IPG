// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "IPGPackageData.generated.h"

class UIPGBundleData;

USTRUCT(BlueprintType)
struct FIPGPackageBundleEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UIPGBundleData> Bundle;

	UPROPERTY(EditAnywhere)
	bool bWritable = true; 

	UPROPERTY(EditAnywhere)
	bool bEditorOnly = false;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "GetValidChunkNames"))
	FName ChunkName; 
};


/**
 * 
 */
UCLASS(BlueprintType)
class IPG_API UIPGPackageData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Build")
	FString PackageName; 

	UPROPERTY(EditAnywhere, Category = "Build", meta = (MultiLine = "true"))
	FString PackageDescription;

	UPROPERTY(EditAnywhere, Category = "Chunks")
	TArray<FName> Chunks;

	UPROPERTY(EditAnywhere, Category = "Chunks")
	FString DefaultBundleForNewPlugins;

	UPROPERTY(EditAnywhere, Category = "Bundles")
	TArray<FIPGPackageBundleEntry> Bundles;

	TArray<FString> GetAllPluginNames(bool bIncludeEditorOnly = true) const; 

	UFUNCTION()
	TArray<FString> GetValidChunkNames() const;

	UFUNCTION(CallInEditor, Category = "Build", meta = (DisplayName = "Export Build Recipe"))
	void ExportBuildRecipe() const;

protected:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};
