// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ToolKits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraEventProxy.h"

class SActionCameraDirectorViewport;
class IDetailsView;
class SActionCameraTimelineTrack;
class UActionCameraDirectorAsset;

class FActionCameraDirectorEditorToolkit : public FAssetEditorToolkit, public FGCObject
{
public:
	void InitCameraDirector(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InToolkitHost, UActionCameraDirectorAsset* InAsset);

	/* FAssetEditorToolKit Interface */
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	/* FAssetEditorToolKit Interface End */

	/* FGCObject Interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	/* FGCObject Interface End */

	void RecalculateClipTimes(); 

	void SetSelectedStep(int32 StepIndex);

	int32 GetSelectedStepIndex() const;

	UActionCameraDirectorAsset* GetAsset() const;

	TSharedPtr<SActionCameraDirectorViewport> GetViewportWidget() const;

private:
	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Timeline(const FSpawnTabArgs& Args);

	void OnAssetPropertyChanged(const FPropertyChangedEvent& Event);
	void OnStepProxyChanged();

	TObjectPtr<UActionCameraDirectorAsset> EditingAsset = nullptr;
	TObjectPtr<UActionCameraEventProxy> SelectedEventProxy = nullptr;

	TSharedPtr<IDetailsView> AssetDetailsView;
	TSharedPtr<IDetailsView> DetailsView;   

	TSharedPtr<SActionCameraDirectorViewport> ViewportWidget;
	TSharedPtr<SActionCameraTimelineTrack> TimelineWidget;

	int32 SelectedStepIndex = INDEX_NONE;

	static const FName ViewportTabId;
	static const FName DetailsTabId;
	static const FName TimelineTabId;
};