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

struct FResolvedClip
{
public:
	UAnimSequenceBase* AnimSequenceBase = nullptr;

	float LocalTime = 0.f;
};

class FActionCameraDirectorViewportClient : public FEditorViewportClient
{
public:
	FActionCameraDirectorViewportClient(
		TWeakPtr<FActionCameraDirectorEditorToolkit> InToolkit,
		FPreviewScene* InPreviewScene,
		const TSharedRef<SActionCameraDirectorViewport>& InViewportWidget);
	
	virtual void Tick(float DeltaSeconds) override;
	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

	void RefreshCameraForTime(float Time); 

	FResolvedClip ResolveClipAtTime(float GlobalTime) const;

	/* Playing */
	bool IsPlaying() const;
	void SetPlaying(bool bNewPlaying);

	/* Looping */
	bool IsLooping() const; 
	void SetLooping(bool bNewLooping);

	/* Playback Time */
	float GetPlaybackTime() const;
	void SetPlaybackTime(float NewPlaybackTime);

	/* Max Playback Time */
	float GetMaxPlaybackTime() const;
	void SetMaxPlaybackTime(float NewMaxPlaybackTime);

private:
	void UpdatePreviewAnimation(float Time);

	UCameraRigAsset* ResolveActiveCameraRig(float Time) const; 
	UCameraVariableCollection* ResolveActiveVariableCollection(float Time) const;

	void ApplyVariableOverridesForTime(float Time);

	UCameraRigAsset* LastAppliedRig = nullptr;

	TWeakPtr<FActionCameraDirectorEditorToolkit> ToolkitPtr;

	/* Tick */
	FActorComponentTickFunction PreviewMeshTickFunction;
	FActorComponentTickFunction PreviewCameraTickFunction;

	/* Preview Component */
	UDebugSkelMeshComponent* PreviewMeshComponent = nullptr;
	UGameplayCameraRigComponent* PreviewCameraComponent = nullptr;

	AActionCameraPreviewCharacter* PreviewCharacter = nullptr;
	APlayerController* DummyController = nullptr;

	/* Playing */
	bool bPlaying = false;

	/* Looping */
	bool bLooping = false;

	/* Playback Time */
	float PlaybackTime = 0.f;

	/* Max Playback Time */
	float MaxPlaybackTime = 0.f;
};