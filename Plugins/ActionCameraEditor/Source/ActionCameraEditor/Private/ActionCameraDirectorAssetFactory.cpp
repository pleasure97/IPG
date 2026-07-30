// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionCameraDirectorAssetFactory.h"
#include "ActionCameraDirectorAsset.h"

UActionCameraDirectorAssetFactory::UActionCameraDirectorAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UActionCameraDirectorAsset::StaticClass();
}

UObject* UActionCameraDirectorAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UActionCameraDirectorAsset>(InParent, InClass, InName, Flags);
}
