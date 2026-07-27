// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SLeafWidget.h"

class FActionCameraDirectorEditorToolkit;

class SActionCameraTimelineTrack : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SActionCameraTimelineTrack) {}
        SLATE_ARGUMENT(TWeakPtr<FActionCameraDirectorEditorToolkit>, Toolkit)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs); 

    void ExtendTimelineBy(float ExtraSeconds);

protected:
    /* SWidget Interface */
    virtual int32 OnPaint(
        const FPaintArgs& Args, 
        const FGeometry& AllottedGeometry, 
        const FSlateRect& MyCullingRect, 
        FSlateWindowElementList& OutDrawElements, 
        int32 LayerId, 
        const FWidgetStyle& InWidgetStyle, 
        bool bParentEnabled) const override;

    virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

    virtual FVector2D ComputeDesiredSize(float) const override;

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
    virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
    enum class EEdgeHit : uint8
    {
        None, 
        Left,
        Right,
        Body
    };

    enum class EDragMode : uint8
    {
        None,
        Move,
        ResizeLeft,
        ResizeRight,
        MoveEvent
    };

    float PixelToTime(float LocalX) const; 
    float TimeToPixel(float Time) const; 

    float GetMaxTime() const;
    float GetLivePlayheadTime() const; 

    int32 HitTestStep(float LocalX, EEdgeHit& OutEdge) const; 
    int32 HitTestEvent(float LocalX) const; 
    void ShowEventContextMenu(const FVector2D& ScreenPosition, float ClickedTime, int32 HitEventIndex); 
    void AddEventAt(float Time); 
    void DeleteEvent(int32 EventIndex);
    void RenameEvent(int32 EventIndex, const FVector2D& ScreenPosition);

    /* Scrub */
    void ScrubToPixel(float LocalX);
    bool bScrubbing = false;

    /* Drag */
    EDragMode DragMode = EDragMode::None;
    int32 DraggedStepIndex = INDEX_NONE;
    int32 DraggedEventIndex = INDEX_NONE;
    float DragStartMouseX = 0.f;
    float DragStartStepTime = 0.f;
    float DragStartStepDuration = 0.f;

    float PlayheadTime = 0.f;
    float ManualExtensionSeconds = 0.f;

    static constexpr float PixelsPerSecond = 120.f;
    static constexpr float RulerHeight = 24.f;
    static constexpr float TrackRowHeight = 48.f;
    static constexpr float EdgeGrabPixels = 6.f;
    static constexpr float MinDuration = 0.05f;
    static constexpr float EventRowHeight = 28.f;
    static constexpr float EventMarkerHalfWidth = 6.f;
    static constexpr float PlayheadGrabPixels = 6.f;

    TWeakPtr<FActionCameraDirectorEditorToolkit> ToolkitPtr;
};