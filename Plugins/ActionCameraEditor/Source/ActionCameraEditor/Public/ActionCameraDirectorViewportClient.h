// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EditorViewportClient.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraDirectorEditorToolkit.h"

class FActionCameraDirectorEditorToolkit;
class FPreviewScene;
class SActionCameraDirectorViewport;
class UDebugSkelMeshComponent;
class UGameplayCameraComponent;
class USingleCameraDirector;
class UCameraAsset;

class FActionCameraDirectorViewportClient : public FEditorViewportClient
{
public:
	FActionCameraDirectorViewportClient(
		TWeakPtr<FActionCameraDirectorEditorToolkit> InToolkit,
		FPreviewScene* InPreviewScene,
		const TSharedRef<SActionCameraDirectorViewport>& InViewportWidget);
	
	virtual void Tick(float DeltaSeconds) override;

	void RefreshCameraForSelectedStep(); 

	/* Playing */
	bool IsPlaying() const;
	void SetPlaying(bool bNewPlaying);
	bool IsLooping() const; 
	void SetLooping(bool bNewLooping);
	float GetPlaybackTime() const;
	void SetPlaybackTime(float NewPlaybackTime);
	float GetMaxPlaybackTime() const;
	void SetMaxPlaybackTime(float NewMaxPlaybackTime);

private:
	TWeakPtr<FActionCameraDirectorEditorToolkit> ToolkitPtr;

	UDebugSkelMeshComponent* PreviewMeshComponent = nullptr;
	UGameplayCameraComponent* PreviewCameraComponent = nullptr;
	USingleCameraDirector* PreviewDirector = nullptr; 
	UCameraAsset* PreviewCameraAsset = nullptr;

	/* Playing*/
	bool bPlaying = false;
	bool bLooping = false;
	float PlaybackTime = 0.f;
	float MaxPlaybackTime = 0.f;
};