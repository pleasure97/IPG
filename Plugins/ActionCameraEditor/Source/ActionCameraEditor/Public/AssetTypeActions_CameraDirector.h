// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class UActionCameraDirectorAsset;

class FAssetTypeActions_CameraDirector : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_CameraDirector(EAssetTypeCategories::Type InCategory);

	/* IAssetTypeActions Interface */
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual uint32 GetCategories() override;

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	/* IAssetTypeActions Interface End */

private:
	EAssetTypeCategories::Type Category;
};