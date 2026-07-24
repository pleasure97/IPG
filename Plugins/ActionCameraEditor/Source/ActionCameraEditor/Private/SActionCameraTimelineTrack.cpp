// Copyright Epic Games, Inc. All Rights Reserved.

#include "SActionCameraTimelineTrack.h"
#include "ActionCameraDirectorEditorToolkit.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraDirectorViewport.h"
#include "ActionCameraDirectorViewportClient.h"

void SActionCameraTimelineTrack::Construct(const FArguments& InArgs)
{
	ToolkitPtr = InArgs._Toolkit;
}

void SActionCameraTimelineTrack::SetPlayheadTime(float NewPlayheadTime)
{
	PlayheadTime = NewPlayheadTime;
}

int32 SActionCameraTimelineTrack::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush("WhiteBrush");
	const FSlateFontInfo SmallFont = FAppStyle::GetFontStyle("PropertyWindow.NormalFont");

	// Background
	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(),
		WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.05f, 0.05f, 0.05f, 1.0f));

	// Ruler (vertical lines at 1-second intervals + labels)
	const float MaxTime = GetMaxTime(); 
	for (int32 Sec = 0; Sec <= FMath::CeilToInt(MaxTime); ++Sec)
	{
		const float X = TimeToPixel(static_cast<float>(Sec));

		TArray<FVector2D> LinePoints;

		LinePoints.Add(FVector2D(X, 0.0f));
		LinePoints.Add(FVector2D(X, Size.Y));

		FSlateDrawElement::MakeLines(
			OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(),
			LinePoints, ESlateDrawEffect::None, FLinearColor(1, 1, 1, 0.08f));

		FSlateDrawElement::MakeText(
			OutDrawElements, LayerId + 2, AllottedGeometry.ToOffsetPaintGeometry(FVector2D(X + 2.0f, 2.0f)),
			FString::Printf(TEXT("%ds"), Sec), SmallFont, ESlateDrawEffect::None,
			FLinearColor(1, 1, 1, 0.4f));
	}

	// Step Blocks 
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			const int32 SelectedIndex = Toolkit->GetSelectedStepIndex();
			const TArray<FActionCameraStep>& Steps = Asset->CameraSteps;

			for (int32 i = 0; i < Steps.Num(); ++i)
			{
				const FActionCameraStep& Step = Steps[i];
				const float StartPx = TimeToPixel(Step.StartTime);
				const float EndPx = TimeToPixel(Step.StartTime + Step.Duration);
				const float BlockWidth = FMath::Max(EndPx - StartPx, 2.0f);

				const bool bSelected = (i == SelectedIndex);
				const FLinearColor BlockColor = bSelected
					? FLinearColor(0.90f, 0.55f, 0.15f, 1.0f)
					: FLinearColor(0.20f, 0.55f, 0.75f, 1.0f);

				const FGeometry BlockGeometry = AllottedGeometry.MakeChild(
					FVector2D(BlockWidth, TrackRowHeight - 4.0f),
					FSlateLayoutTransform(FVector2D(StartPx, RulerHeight + 2.0f)));

				FSlateDrawElement::MakeBox(
					OutDrawElements, LayerId + 3, BlockGeometry.ToPaintGeometry(),
					WhiteBrush, ESlateDrawEffect::None, BlockColor);

				// The selected block is highlighted by drawing a border one more time
				if (bSelected)
				{
					TArray<FVector2D> BorderPoints;
					BorderPoints.Add(FVector2D(0, 0));
					BorderPoints.Add(FVector2D(BlockWidth, 0));
					BorderPoints.Add(FVector2D(BlockWidth, TrackRowHeight - 4.0f));
					BorderPoints.Add(FVector2D(0, TrackRowHeight - 4.0f));
					BorderPoints.Add(FVector2D(0, 0));

					FSlateDrawElement::MakeLines(
						OutDrawElements, LayerId + 4, BlockGeometry.ToPaintGeometry(),
						BorderPoints, ESlateDrawEffect::None, FLinearColor::White, true, 2.0f);
				}

				// Skip if too narrow to avoid text overlap
				if (BlockWidth > 24.0f) 
				{
					FSlateDrawElement::MakeText(
						OutDrawElements, LayerId + 5,
						BlockGeometry.ToOffsetPaintGeometry(FVector2D(4.0f, 4.0f)),
						Step.StepName.IsNone() ? FString::Printf(TEXT("Step %d"), i) : Step.StepName.ToString(),
						SmallFont, ESlateDrawEffect::None, FLinearColor::White);
				}
			}
		}
	}

	float LivePlayheadTime = 0.0f;
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (TSharedPtr<SActionCameraDirectorViewport> Viewport = Toolkit->GetViewportWidget())
		{
			if (TSharedPtr<FActionCameraDirectorViewportClient> Client = Viewport->GetTypedViewportClient())
			{
				LivePlayheadTime = Client->GetPlaybackTime();
			}
		}
	}

	// Playhead
	const float PlayheadX = TimeToPixel(PlayheadTime);
	TArray<FVector2D> PlayheadPoints;
	PlayheadPoints.Add(FVector2D(PlayheadX, 0.0f));
	PlayheadPoints.Add(FVector2D(PlayheadX, Size.Y));

	FSlateDrawElement::MakeLines(
		OutDrawElements, LayerId + 6, AllottedGeometry.ToPaintGeometry(),
		PlayheadPoints, ESlateDrawEffect::None, FLinearColor(1.0f, 0.2f, 0.2f, 0.9f), true, 2.0f);

	return LayerId + 7;
}

void SActionCameraTimelineTrack::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SLeafWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D SActionCameraTimelineTrack::ComputeDesiredSize(float) const
{
	const float Width = TimeToPixel(GetMaxTime()) + 40.f; 
	return FVector2D(Width, RulerHeight + TrackRowHeight * 8.f);
}

FReply SActionCameraTimelineTrack::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Only Left Mouse Button
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()); 

	// Check if it's scrubbing
	if (LocalPosition.Y < RulerHeight)
	{
		bScrubbing = true;
		ScrubToPixel(LocalPosition.X);
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	EEdgeHit Edge; 
	const int32 HitIndex = HitTestStep(LocalPosition.X, Edge);

	if (HitIndex == INDEX_NONE)
	{
		return FReply::Unhandled(); 
	}

	// Check if action camera editor toolkit is valid
	TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin(); 
	if (!Toolkit.IsValid() || !Toolkit->GetAsset())
	{
		return FReply::Unhandled(); 
	}

	Toolkit->SetSelectedStep(HitIndex); 

	DraggedStepIndex = HitIndex;
	DragStartMouseX = LocalPosition.X;

	const FActionCameraStep& ActiveCameraStep = Toolkit->GetAsset()->CameraSteps[HitIndex];
	DragStartStepTime = ActiveCameraStep.StartTime;
	DragStartStepDuration = ActiveCameraStep.Duration;

	switch (Edge)
	{
	case EEdgeHit::Left:
	{
		DragMode = EDragMode::ResizeLeft;
		break;
	}
	case EEdgeHit::Right:
	{
		DragMode = EDragMode::ResizeRight;
		break;
	}
	default:
		DragMode = EDragMode::Move;
		break;
	}

	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SActionCameraTimelineTrack::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Scrub Timeline Track
	if (bScrubbing)
	{
		const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		ScrubToPixel(LocalPos.X);
		return FReply::Handled();
	}

	if (DragMode == EDragMode::None || DraggedStepIndex == INDEX_NONE)
	{
		return FReply::Unhandled();
	}

	TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid() || !Toolkit->GetAsset())
	{
		return FReply::Unhandled();
	}

	UActionCameraDirectorAsset* Asset = Toolkit->GetAsset();
	if (!Asset->CameraSteps.IsValidIndex(DraggedStepIndex))
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const float DeltaPixels = LocalPosition.X - DragStartMouseX;
	const float DeltaTime = DeltaPixels / PixelsPerSecond;

	FActionCameraStep& ActionCameraStep = Asset->CameraSteps[DraggedStepIndex];

	Asset->Modify();

	switch (DragMode)
	{
	case EDragMode::Move:
	{
		ActionCameraStep.StartTime = FMath::Max(DragStartStepTime + DeltaTime, 0.0f);
		break;
	}
	case EDragMode::ResizeLeft:
	{
		const float OriginalEnd = DragStartStepTime + DragStartStepDuration;
		const float NewStart = FMath::Clamp(DragStartStepTime + DeltaTime, 0.0f, OriginalEnd - MinDuration);
		ActionCameraStep.StartTime = NewStart;
		ActionCameraStep.Duration = OriginalEnd - NewStart;
		break;
	}
	case EDragMode::ResizeRight:
	{
		ActionCameraStep.Duration = FMath::Max(DragStartStepDuration + DeltaTime, MinDuration);
		break;
	}
	default:
		break;
	}

	return FReply::Handled(); 
}

FReply SActionCameraTimelineTrack::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Process Scurbbing
	if (bScrubbing)
	{
		bScrubbing = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	// Process Drag 
	if (DragMode == EDragMode::None)
	{
		return FReply::Unhandled();
	}

	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			Asset->MarkPackageDirty();
		}

		Toolkit->SetSelectedStep(DraggedStepIndex);
	}

	DragMode = EDragMode::None;
	DraggedStepIndex = INDEX_NONE;

	return FReply::Handled().ReleaseMouseCapture();
}

void SActionCameraTimelineTrack::OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	DragMode = EDragMode::None;
	DraggedStepIndex = INDEX_NONE;
}

FCursorReply SActionCameraTimelineTrack::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	if (DragMode == EDragMode::ResizeLeft || DragMode == EDragMode::ResizeRight)
	{
		return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(CursorEvent.GetScreenSpacePosition());
	EEdgeHit EdgeHit;

	if (HitTestStep(LocalPosition.X, EdgeHit) != INDEX_NONE)
	{
		if (EdgeHit == EEdgeHit::Left || EdgeHit == EEdgeHit::Right)
		{
			return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
		}
		return FCursorReply::Cursor(EMouseCursor::GrabHand);
	}

	return FCursorReply::Unhandled();
}

float SActionCameraTimelineTrack::PixelToTime(float LocalX) const
{
	return FMath::Max(LocalX / PixelsPerSecond, 0.f);
}

float SActionCameraTimelineTrack::TimeToPixel(float Time) const
{
	return Time * PixelsPerSecond;
}

float SActionCameraTimelineTrack::GetMaxTime() const
{
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			if (UAnimSequence* Anim = Asset->ReferenceAnimation.LoadSynchronous())
			{
				return FMath::Max(Anim->GetPlayLength(), 1.0f);
			}
		}
	}

	return 5.f;
}

int32 SActionCameraTimelineTrack::HitTestStep(float LocalX, EEdgeHit& OutEdge) const
{
	OutEdge = EEdgeHit::None;

	TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid() || !Toolkit->GetAsset())
	{
		return INDEX_NONE;
	}

	const TArray<FActionCameraStep>& Steps = Toolkit->GetAsset()->CameraSteps;

	for (int32 i = Steps.Num() - 1; i >= 0; --i)
	{
		const float StartPixel = TimeToPixel(Steps[i].StartTime);
		const float EndPixel = TimeToPixel(Steps[i].StartTime + Steps[i].Duration);

		if (LocalX < StartPixel - EdgeGrabPixels || LocalX > EndPixel + EdgeGrabPixels)
		{
			continue;
		}

		if (FMath::Abs(LocalX - StartPixel) <= EdgeGrabPixels)
		{
			OutEdge = EEdgeHit::Left;
		}
		else if (FMath::Abs(LocalX - EndPixel) <= EdgeGrabPixels)
		{
			OutEdge = EEdgeHit::Right;
		}
		else
		{
			OutEdge = EEdgeHit::Body;
		}
		return i;
	}
	return INDEX_NONE;
}

void SActionCameraTimelineTrack::ScrubToPixel(float LocalX)
{
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (TSharedPtr<SActionCameraDirectorViewport> Viewport = Toolkit->GetViewportWidget())
		{
			if (TSharedPtr<FActionCameraDirectorViewportClient> Client = Viewport->GetTypedViewportClient())
			{
				Client->SetPlaybackTime(PixelToTime(LocalX));
			}
		}
	}
}
