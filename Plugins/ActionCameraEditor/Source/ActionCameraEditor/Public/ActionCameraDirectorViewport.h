// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SEditorViewport.h"

class FActionCameraDirectorViewportClient;
class FActionCameraDirectorEditorToolkit;

class SActionCameraDirectorViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SActionCameraDirectorViewport) {}
		SLATE_ARGUMENT(TWeakPtr<FActionCameraDirectorEditorToolkit>, Toolkit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SActionCameraDirectorViewport(); 

	void RefreshCameraForSelectedStep(); 

	TSharedPtr<FActionCameraDirectorViewportClient> GetTypedViewportClient() const;

protected:
	/* SEditorViewport Interface */
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
	virtual void PopulateViewportOverlays(TSharedRef<SOverlay> Overlay) override; 
	virtual void BindCommands() override;
	virtual EVisibility GetTransformToolbarVisibility() const override;
	/* SEditorViewport Interface Ends */

private:
	/* Callbacks for Playback Control UI */
	FReply OnPlayPauseClicked();
	const FSlateBrush* GetPlayPauseIcon() const;
	FText GetPlayPauseToolTip() const; 

	FReply OnRewindClicked(); 

	ECheckBoxState GetLoopCheckState() const;

	void OnLoopCheckStateChanged(ECheckBoxState NewState);

	/* Scrub Slider */
	float GetScrubValue() const;
	void OnScrubValueChanged(float NewValue); 
	void OnScrubBegin(); 
	void OnScrubEnd(float NewValue); 

	FText GetTimeLabelText() const;

	bool bWasPlayingBeforeScrub = false; 

	TWeakPtr<FActionCameraDirectorEditorToolkit> ToolkitPtr;
	TSharedPtr<FPreviewScene> PreviewScene;
	TSharedPtr<FActionCameraDirectorViewportClient> TypedViewportClient;
};