// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGGameFeatureTypeData.h"

FString UIPGGameFeatureTypeData::GetDisplayName() const
{
	return TypeName.IsEmpty() ? GetName() : TypeName;
}

FPrimaryAssetId UIPGGameFeatureTypeData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("IPGGameFeatureTypeData"), GetFName());
}
