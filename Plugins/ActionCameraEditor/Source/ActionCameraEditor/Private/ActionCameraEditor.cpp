// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionCameraEditor.h"
#include "AssetToolsModule.h"
#include "AssetTypeActions_CameraDirector.h"

#define LOCTEXT_NAMESPACE "ActionCameraEditor"

void FActionCameraEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	EAssetTypeCategories::Type Category = AssetTools.RegisterAdvancedAssetCategory(FName("ActionCamera"), LOCTEXT("ActionCameraCategory", "Action Camera"));
	CameraDirectorAssetTypeActions = MakeShared<FAssetTypeActions_CameraDirector>(Category);
	
	// Register asset type actions in asset tools
	AssetTools.RegisterAssetTypeActions(CameraDirectorAssetTypeActions.ToSharedRef());
}

void FActionCameraEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		// Unregister asset type actions in asset tools
		AssetTools.UnregisterAssetTypeActions(CameraDirectorAssetTypeActions.ToSharedRef());
	}

	CameraDirectorAssetTypeActions.Reset();
}

#undef LOCTEXT_NAMESPACE // ActionCameraEditor
	
IMPLEMENT_MODULE(FActionCameraEditorModule, ActionCameraEditor)