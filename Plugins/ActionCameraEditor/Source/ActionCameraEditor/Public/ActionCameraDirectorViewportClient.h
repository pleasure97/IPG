// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EditorViewportClient.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraDirectorEditorToolkit.h"

class FActionCameraDirectorEditorToolkit;
class FPreviewScene;
class SActionCameraDirectorViewport;
class UDebugSkelMeshComponent;
class UGameplayCameraRigComponent;
class AActionCameraPreviewCharacter;
class APlayerController;
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
	void UpdatePreviewAnimation(float Time);

	TWeakPtr<FActionCameraDirectorEditorToolkit> ToolkitPtr;

	FActorComponentTickFunction PreviewMeshTickFunction;
	FActorComponentTickFunction PreviewCameraTickFunction;

	UDebugSkelMeshComponent* PreviewMeshComponent = nullptr;
	UGameplayCameraRigComponent* PreviewCameraComponent = nullptr;

	AActionCameraPreviewCharacter* PreviewCharacter = nullptr;
	APlayerController* DummyController = nullptr;

	/* Playing*/
	bool bPlaying = false;
	bool bLooping = false;
	float PlaybackTime = 0.f;
	float MaxPlaybackTime = 0.f;
};