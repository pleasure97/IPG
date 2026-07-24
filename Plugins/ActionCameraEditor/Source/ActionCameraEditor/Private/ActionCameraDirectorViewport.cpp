// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionCameraDirectorViewport.h"
#include "ActionCameraDirectorViewportClient.h"
#include "SEditorViewportToolBarMenu.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"
#include "Animation/AnimSequence.h"

#define LOCTEXT_NAMESPACE "ActionCameraEditor"
void SActionCameraDirectorViewport::Construct(const FArguments& InArgs)
{
    ToolkitPtr = InArgs._Toolkit;

    // Preview-only world
    PreviewScene = MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues()
        .SetLightBrightness(3.0f)
        .SetSkyBrightness(1.0f)
        .SetCreatePhysicsScene(false)
        .AllowAudioPlayback(false));

    SEditorViewport::Construct(SEditorViewport::FArguments());
}

SActionCameraDirectorViewport::~SActionCameraDirectorViewport()
{
}

void SActionCameraDirectorViewport::RefreshCameraForSelectedStep()
{
    if (TypedViewportClient.IsValid())
    {
        TypedViewportClient->RefreshCameraForSelectedStep();
    }
}

TSharedPtr<FActionCameraDirectorViewportClient> SActionCameraDirectorViewport::GetTypedViewportClient() const
{
	return TypedViewportClient;
}

/* SEditorViewport Interface */
TSharedRef<FEditorViewportClient> SActionCameraDirectorViewport::MakeEditorViewportClient()
{
    // Objects are created in the order of Toolkit -> ViewportWidget -> ViewportClient,
    // and SharedThis(this) is passed here so that the client holds a weak reference to this widget
    TypedViewportClient = MakeShared<FActionCameraDirectorViewportClient>(ToolkitPtr, PreviewScene.Get(), SharedThis(this)); 
    return TypedViewportClient.ToSharedRef();
}

void SActionCameraDirectorViewport::PopulateViewportOverlays(TSharedRef<SOverlay> Overlay)
{
    SEditorViewport::PopulateViewportOverlays(Overlay); 

    // Place playback controls in the form of a semi-transparent bar at the bottom of the viewport.
    Overlay->AddSlot()
        .VAlign(VAlign_Bottom)
        .HAlign(HAlign_Fill)
        .Padding(FMargin(8.f, 0.f, 8.f, 8.f))
        [
            SNew(SBorder)
                .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
                .Padding(FMargin(6.f))
                [
                    SNew(SHorizontalBox)

                        // Rewind
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(2.f)
                        [
                            SNew(SButton)
                                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                                .ToolTipText(LOCTEXT("Rewind", "Rewind (K)"))
                                .OnClicked(this, &SActionCameraDirectorViewport::OnRewindClicked)
                                [
                                    SNew(SImage)
                                        .Image(FAppStyle::GetBrush("Animation.Backward"))
                                ]
                        ]

                    // Play / Pause
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(2.f)
                        [
                            SNew(SButton)
                                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                                .ToolTipText(this, &SActionCameraDirectorViewport::GetPlayPauseToolTip)
                                .OnClicked(this, &SActionCameraDirectorViewport::OnPlayPauseClicked)
                                [
                                    SNew(SImage)
                                        .Image(this, &SActionCameraDirectorViewport::GetPlayPauseIcon)
                                ]
                        ]

                    // Loop toggle
                    + SHorizontalBox::Slot()
                        .AutoWidth()
                        .VAlign(VAlign_Center)
                        .Padding(2.f)
                        [
                            SNew(SCheckBox)
                                .Style(FAppStyle::Get(), "ToggleButtonCheckbox")
                                .ToolTipText(LOCTEXT("Loop", "Loop (L)"))
                                .IsChecked(this, &SActionCameraDirectorViewport::GetLoopCheckState)
                                .OnCheckStateChanged(this, &SActionCameraDirectorViewport::OnLoopCheckStateChanged)
                                [
                                    SNew(SImage)
                                        .Image(FAppStyle::GetBrush("Animation.Loop.Enabled"))
                                ]
                        ]

                    // Scrub Slider 
                    + SHorizontalBox::Slot()
                        .FillWidth(1.f)
                        .VAlign(VAlign_Center)
                        .Padding(8.f, 2.f)
                        [
                            SNew(SSlider)
                                .Value(this, &SActionCameraDirectorViewport::GetScrubValue)
                                .OnValueChanged(this, &SActionCameraDirectorViewport::OnScrubValueChanged)
                                .OnMouseCaptureBegin(this, &SActionCameraDirectorViewport::OnScrubBegin)
                                .OnMouseCaptureEnd_Lambda(
                                    [this]()
                                    {
                                        OnScrubEnd(GetScrubValue());
                                    })
                        ]
                ]
        ];
}

void SActionCameraDirectorViewport::BindCommands()
{
    SEditorViewport::BindCommands();

    // TODO - If you need a custom viewport command (e.g., the Rewind shortcut), enter here
}

EVisibility SActionCameraDirectorViewport::GetTransformToolbarVisibility() const
{
    return EVisibility();
}

/* Callbacks for Playback Control UI */
FReply SActionCameraDirectorViewport::OnPlayPauseClicked()
{
    if (TypedViewportClient.IsValid())
    {
        const bool bNowPlaying = !TypedViewportClient->IsPlaying(); 
        TypedViewportClient->SetPlaying(bNowPlaying);
    }
    return FReply::Handled();
}

const FSlateBrush* SActionCameraDirectorViewport::GetPlayPauseIcon() const
{
    const bool bIsPlaying = TypedViewportClient.IsValid() && TypedViewportClient->IsPlaying();
    return FAppStyle::GetBrush(bIsPlaying ? "Animation.Pause" : "Animation.Forward");
}

FText SActionCameraDirectorViewport::GetPlayPauseToolTip() const
{
    const bool bIsPlaying = TypedViewportClient.IsValid() && TypedViewportClient->IsPlaying();
    return bIsPlaying ? LOCTEXT("Pause", "Pause (Space)") : LOCTEXT("Play", "Play (Space)");
}

FReply SActionCameraDirectorViewport::OnRewindClicked()
{
    if (TypedViewportClient.IsValid())
    {
        TypedViewportClient->SetPlaybackTime(0.0f);
    }
    return FReply::Handled();
}

ECheckBoxState SActionCameraDirectorViewport::GetLoopCheckState() const
{
    const bool bLooping = TypedViewportClient.IsValid() && TypedViewportClient->IsLooping();
    return bLooping ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SActionCameraDirectorViewport::OnLoopCheckStateChanged(ECheckBoxState NewState)
{
    if (TypedViewportClient.IsValid())
    {
        TypedViewportClient->SetLooping(NewState == ECheckBoxState::Checked);
    }
}

/* Scrub Slider */
float SActionCameraDirectorViewport::GetScrubValue() const
{
    if (!TypedViewportClient.IsValid())
    {
        return 0.f;
    }

    const float MaxTime = TypedViewportClient->GetMaxPlaybackTime();
    if (MaxTime <= KINDA_SMALL_NUMBER)
    {
        return 0.f;
    }

    return TypedViewportClient->GetPlaybackTime() / MaxTime;
}

void SActionCameraDirectorViewport::OnScrubValueChanged(float NewValue)
{
    if (TypedViewportClient.IsValid())
    {
        const float MaxTime = TypedViewportClient->GetMaxPlaybackTime();
        TypedViewportClient->SetPlaybackTime(NewValue * MaxTime);
    }
}

void SActionCameraDirectorViewport::OnScrubBegin()
{
    if (TypedViewportClient.IsValid())
    {
        // Pause autoplay while dragging to prevent the slider value and playback timer from overwriting each other
        // Restore the original state when dragging ends
        bWasPlayingBeforeScrub = TypedViewportClient->IsPlaying();
        TypedViewportClient->SetPlaying(false);
    }
}

void SActionCameraDirectorViewport::OnScrubEnd(float NewValue)
{
    if (TypedViewportClient.IsValid() && bWasPlayingBeforeScrub)
    {
        TypedViewportClient->SetPlaying(true);
    }
}

FText SActionCameraDirectorViewport::GetTimeLabelText() const
{
    if (!TypedViewportClient.IsValid())
    {
        return FText::GetEmpty();
    }

    return FText::Format(
        LOCTEXT("TimeLabelFormat", "{0} / {1}"),
        FText::AsNumber(TypedViewportClient->GetPlaybackTime()),
        FText::AsNumber(TypedViewportClient->GetMaxPlaybackTime()));
}
#undef LOCTEXT_NAMESPACE // ActionCameraEditor