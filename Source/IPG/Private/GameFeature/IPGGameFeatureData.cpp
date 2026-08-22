// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGGameFeatureData.h"
#include "GameFeature/IPGGameFeatureTypeData.h"
#include "UObject/ObjectSaveContext.h"

#if WITH_EDITOR
void UIPGGameFeatureData::PostLoad()
{
	Super::PostLoad(); 

	// Since the game feature type data may have been modified, realign the data at load time.
	UpdateGameFeatureTypeData();
}

void UIPGGameFeatureData::PreSave(FObjectPreSaveContext SaveContext)
{
	// Synchronize the serialized value to always match the game feature type data
	if (!SaveContext.IsCooking() && !SaveContext.IsProceduralSave())
	{
		UpdateGameFeatureTypeData();
	}

	Super::PreSave(SaveContext);
}

void UIPGGameFeatureData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent); 

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UIPGGameFeatureData, GameFeatureType))
	{
		UpdateGameFeatureTypeData();
	}
}

bool UIPGGameFeatureData::CanEditChange(const FProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty))
	{
		return false;
	}

	if (InProperty == nullptr)
	{
		return true;
	}

	// If the game feature type is specified, it is automatically managed
	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UIPGGameFeatureTypeData, PrimaryAssetTypesToScan))
	{
		return GameFeatureType.IsNull();
	}

	return true;
}
#endif // WITH_EDITOR

void UIPGGameFeatureData::UpdateGameFeatureTypeData()
{
	const UIPGGameFeatureTypeData* GameFeatureTypeData = GameFeatureType.LoadSynchronous(); 
	if (!IsValid(GameFeatureTypeData))
	{
		return;
	}

	const FString PluginRoot = GetPluginRoot(); 

	TArray<FPrimaryAssetTypeInfo> PrimaryAssetTypesFromGFTP = GameFeatureTypeData->PrimaryAssetTypesToScan;

	for (FPrimaryAssetTypeInfo& PrimaryAssetTypeFromGFTP : PrimaryAssetTypesFromGFTP)
	{
		for (auto& Directory : PrimaryAssetTypeFromGFTP.GetDirectories())
		{
			Directory.Path = Directory.Path.Replace(TEXT("{PluginRoot}"), *PluginRoot);
		}
	}

	PrimaryAssetTypesToScan = MoveTemp(PrimaryAssetTypesFromGFTP);

	UE_LOG(LogTemp, Log,
		TEXT("Synchronized the asset manager settings for '%s' from type '%s'"), *GetName(), *GameFeatureTypeData->GetDisplayName());
}

FString UIPGGameFeatureData::GetPluginRoot() const
{
	// Package Name (e.g., /Mode_A/Mode_A)
	FString PackageName = GetPackage()->GetName(); 
	PackageName.RemoveFromStart(TEXT("/")); 

	FString RootName;
	if (!PackageName.Split(TEXT("/"), &RootName, nullptr))
	{
		RootName = PackageName;
	}

	return TEXT("/") + RootName;
}
