// Copyright Epic Games, Inc. All Rights Reserved.

#include "SActionCameraTimelineTrack.h"
#include "ActionCameraDirectorEditorToolkit.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraDirectorViewport.h"
#include "ActionCameraDirectorViewportClient.h"
#include "Widgets/Input/STextEntryPopup.h"

static FLinearColor GetColorForEventName(FName EventName)
{
	static const TArray<FLinearColor> Palette = {
		FLinearColor(0.95f, 0.25f, 0.55f), // 
		FLinearColor(0.25f, 0.65f, 0.95f), // 
		FLinearColor(0.95f, 0.65f, 0.15f), // 
		FLinearColor(0.45f, 0.85f, 0.35f), //
		FLinearColor(0.75f, 0.35f, 0.95f), //
		FLinearColor(0.95f, 0.85f, 0.25f), //
	};
	const uint32 Hash = GetTypeHash(EventName);
	return Palette[Hash % Palette.Num()];
}

void SActionCameraTimelineTrack::Construct(const FArguments& InArgs)
{
	ToolkitPtr = InArgs._Toolkit;
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
	for (int32 Sec = 0; Sec <= FMath::CeilToInt(GetMaxTime()); ++Sec)
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

	// Event Row
	const float EventRowY = RulerHeight + TrackRowHeight + 4.0f;
	FSlateDrawElement::MakeBox(
		OutDrawElements, LayerId + 3,
		AllottedGeometry.ToPaintGeometry(FVector2D(Size.X, EventRowHeight), FSlateLayoutTransform(FVector2D(0, EventRowY))),
		WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.03f, 0.03f, 0.03f, 1.0f));

	// Event Row & Step Blocks 
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			// Event Row 
			for (int32 i = 0; i < Asset->CameraEvents.Num(); ++i)
			{
				const FActionCameraEvent& ActionCameraEvent = Asset->CameraEvents[i];
				const FLinearColor EventColor = GetColorForEventName(ActionCameraEvent.EventName);
				const float CenterY = EventRowY + EventRowHeight * 0.5f;

				if (ActionCameraEvent.Duration > KINDA_SMALL_NUMBER)
				{
					const float StartX = TimeToPixel(ActionCameraEvent.Time);
					const float EndX = TimeToPixel(ActionCameraEvent.Time + ActionCameraEvent.Duration);
					const float BarWidth = FMath::Max(EndX - StartX, 2.0f);

					const FGeometry BarGeometry = AllottedGeometry.MakeChild(
						FVector2D(BarWidth, EventRowHeight - 4.0f),
						FSlateLayoutTransform(FVector2D(StartX, EventRowY + 2.0f)));

					FSlateDrawElement::MakeBox(
						OutDrawElements, LayerId + 5, BarGeometry.ToPaintGeometry(),
						WhiteBrush, ESlateDrawEffect::None, EventColor.CopyWithNewOpacity(0.45f));

					FSlateDrawElement::MakeText(
						OutDrawElements, LayerId + 6,
						BarGeometry.ToOffsetPaintGeometry(FVector2D(4.0f, 3.0f)),
						ActionCameraEvent.EventName.IsNone() ? TEXT("Event") : ActionCameraEvent.EventName.ToString(),
						SmallFont, ESlateDrawEffect::None, EventColor);
				}
				else
				{
					const float CenterX = TimeToPixel(ActionCameraEvent.Time);

					// Diamond Marker
					const float DiamondSize = EventMarkerHalfWidth * 1.4f;  

					const FGeometry DiamondGeometry = AllottedGeometry.MakeChild(
						FVector2D(DiamondSize, DiamondSize),
						FSlateLayoutTransform(FVector2D(CenterX - DiamondSize * 0.5f, CenterY - DiamondSize * 0.5f)));

					FSlateDrawElement::MakeRotatedBox(
						OutDrawElements,
						LayerId + 6,
						DiamondGeometry.ToPaintGeometry(),
						WhiteBrush,
						ESlateDrawEffect::None,
						FMath::DegreesToRadians(45.0f),
						TOptional<FVector2D>(),          
						FSlateDrawElement::RelativeToElement,
						EventColor);

					FSlateDrawElement::MakeText(
						OutDrawElements, LayerId + 6,
						AllottedGeometry.ToOffsetPaintGeometry(FVector2D(CenterX + EventMarkerHalfWidth + 3.0f, EventRowY + 4.0f)),
						ActionCameraEvent.EventName.IsNone() ? TEXT("Event") : ActionCameraEvent.EventName.ToString(),
						SmallFont, ESlateDrawEffect::None, EventColor);
				}
			}

			// Step Blocks
			const int32 SelectedIndex = Toolkit->GetSelectedStepIndex();
			const TArray<FActionCameraClip>& Steps = Asset->CameraSteps;

			for (int32 i = 0; i < Steps.Num(); ++i)
			{
				const FActionCameraClip& Step = Steps[i];
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
						Step.ClipName.IsNone() ? FString::Printf(TEXT("Step %d"), i) : Step.ClipName.ToString(),
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
	const float PlayheadX = TimeToPixel(GetLivePlayheadTime());
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
{;
	const float Width = TimeToPixel(GetMaxTime()) + 40.f; 
	return FVector2D(Width, RulerHeight + TrackRowHeight + EventRowHeight + 8.f);
}

FReply SActionCameraTimelineTrack::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const float EventRowY = RulerHeight + TrackRowHeight + 4.0f;

	// Right Click : If the mouse is in event row, spawn the menu (add / delete / change name)
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (LocalPosition.Y >= EventRowY && LocalPosition.Y <= EventRowY + EventRowHeight)
		{
			const int32 HitEvent = HitTestEvent(LocalPosition.X);
			ShowEventContextMenu(MouseEvent.GetScreenSpacePosition(), PixelToTime(LocalPosition.X), HitEvent);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	// Only Left Mouse Button
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	// Scrub starts near the playhead
	const float PlayheadX = TimeToPixel(GetLivePlayheadTime());
	if (FMath::Abs(LocalPosition.X - PlayheadX) <= PlayheadGrabPixels || LocalPosition.Y < RulerHeight)
	{
		bScrubbing = true;
		ScrubToPixel(LocalPosition.X);
		return FReply::Handled().CaptureMouse(SharedThis(this));
	}

	// Left-click and drag the event marker
	if (LocalPosition.Y >= EventRowY && LocalPosition.Y <= EventRowY + EventRowHeight)
	{
		const int32 HitEvent = HitTestEvent(LocalPosition.X); 
		if (HitEvent != INDEX_NONE)
		{
			DraggedEventIndex = HitEvent;
			DragMode = EDragMode::MoveEvent;
			DragStartMouseX = LocalPosition.X; 

			if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
			{
				DragStartStepTime = Toolkit->GetAsset()->CameraEvents[HitEvent].Time;
			}
			return FReply::Handled().CaptureMouse(SharedThis(this));
		}
		return FReply::Unhandled();
	}

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

	const FActionCameraClip& ActiveCameraClip = Toolkit->GetAsset()->CameraSteps[HitIndex];
	DragStartStepTime = ActiveCameraClip.StartTime;
	DragStartStepDuration = ActiveCameraClip.Duration;

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

	FActionCameraClip& ActionCameraStep = Asset->CameraSteps[DraggedStepIndex];

	Asset->Modify();

	switch (DragMode)
	{
	case EDragMode::Move:
	{
		const float DraggedCenterTime = PixelToTime(LocalPosition.X);
		int32 NewIndex = DraggedStepIndex;
		for (int32 i = 0; i < Asset->CameraSteps.Num(); ++i)
		{
			if (i == DraggedStepIndex)
			{
				continue;
			}
			const float OtherCenter = Asset->CameraSteps[i].StartTime + Asset->CameraSteps[i].Duration * 0.5f;
			if ((DraggedCenterTime > OtherCenter) == (i > DraggedStepIndex))
			{
				NewIndex = i;
			}
			if (NewIndex != DraggedStepIndex)
			{
				Asset->Modify();
				Asset->CameraSteps.Swap(DraggedStepIndex, NewIndex);
				DraggedStepIndex = NewIndex;
				Toolkit->RecalculateClipTimes();
			}
		}
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
	case EDragMode::MoveEvent:
	{
		if (Asset && Asset->CameraEvents.IsValidIndex(DraggedEventIndex))
		{
			Asset->Modify();
			const float MoveEventDeltaTime = (LocalPosition.X - DragStartMouseX) / PixelsPerSecond;
			Asset->CameraEvents[DraggedEventIndex].Time = FMath::Clamp(DragStartStepTime + MoveEventDeltaTime, 0.0f, GetMaxTime());
		}
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
	float MaxTime = 1.0f;

	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			if (Asset->CameraSteps.Num() > 0)
			{
				const FActionCameraClip& LastClip = Asset->CameraSteps.Last();
				MaxTime = LastClip.StartTime + LastClip.Duration;
			}
		}
	}

	return MaxTime + ManualExtensionSeconds;
}

float SActionCameraTimelineTrack::GetLivePlayheadTime() const
{
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (TSharedPtr<SActionCameraDirectorViewport> Viewport = Toolkit->GetViewportWidget())
		{
			if (TSharedPtr<FActionCameraDirectorViewportClient> Client = Viewport->GetTypedViewportClient())
			{
				return Client->GetPlaybackTime();
			}
		}
	}
	return 0.0f;
}

void SActionCameraTimelineTrack::ExtendTimelineBy(float ExtraSeconds)
{
	ManualExtensionSeconds += ExtraSeconds;
	Invalidate(EInvalidateWidgetReason::Layout);
}

int32 SActionCameraTimelineTrack::HitTestStep(float LocalX, EEdgeHit& OutEdge) const
{
	OutEdge = EEdgeHit::None;

	TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid() || !Toolkit->GetAsset())
	{
		return INDEX_NONE;
	}

	const TArray<FActionCameraClip>& Steps = Toolkit->GetAsset()->CameraSteps;

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

int32 SActionCameraTimelineTrack::HitTestEvent(float LocalX) const
{
	TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
	if (!Toolkit.IsValid() || !Toolkit->GetAsset())
	{
		return INDEX_NONE;
	}

	const TArray<FActionCameraEvent>& ActionCameraEvents = Toolkit->GetAsset()->CameraEvents;
	for (int32 i = ActionCameraEvents.Num() - 1; i >= 0; --i)
	{
		const FActionCameraEvent& ActionCameraEvent = ActionCameraEvents[i];

		if (ActionCameraEvent.Duration > KINDA_SMALL_NUMBER)
		{
			const float StartPixel = TimeToPixel(ActionCameraEvent.Time);
			const float EndPixel = TimeToPixel(ActionCameraEvent.Time + ActionCameraEvent.Duration);
			if (LocalX < StartPixel - EdgeGrabPixels || LocalX > EndPixel + EdgeGrabPixels)
			{
				continue;
			}

			return i;
		}
		else
		{
			const float CenterX = TimeToPixel(ActionCameraEvent.Time);
			if (FMath::Abs(LocalX - CenterX) <= EventMarkerHalfWidth + 2.0f)
			{
				return i;
			}
		}
	}

	return INDEX_NONE;
}

void SActionCameraTimelineTrack::ShowEventContextMenu(const FVector2D& ScreenPosition, float ClickedTime, int32 HitEventIndex)
{
	FMenuBuilder MenuBuilder(/*bCloseSelfOnly=*/true, nullptr);

	if (HitEventIndex != INDEX_NONE)
	{
		// "Rename Event" Menu
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Rename Event")),
			FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this, HitEventIndex, ScreenPosition]()
				{
					RenameEvent(HitEventIndex, ScreenPosition);
				})));

		// "Delete Event" Menu
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Delete Event")),
			FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SActionCameraTimelineTrack::DeleteEvent, HitEventIndex)));
	}
	else
	{
		// "Add Event" Menu
		MenuBuilder.AddMenuEntry(
			FText::FromString(TEXT("Add Event Here")),
			FText::GetEmpty(), FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SActionCameraTimelineTrack::AddEventAt, ClickedTime)));
	}

	FSlateApplication::Get().PushMenu(
		SharedThis(this), FWidgetPath(), MenuBuilder.MakeWidget(),
		ScreenPosition, FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

void SActionCameraTimelineTrack::AddEventAt(float Time)
{
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			Asset->Modify();
			FActionCameraEvent NewEvent;
			NewEvent.EventName = TEXT("NewEvent");
			NewEvent.Time = Time;
			Asset->CameraEvents.Add(NewEvent);
			Asset->MarkPackageDirty();
		}
	}
}

void SActionCameraTimelineTrack::DeleteEvent(int32 EventIndex)
{
	if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
	{
		if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
		{
			if (Asset->CameraEvents.IsValidIndex(EventIndex))
			{
				Asset->Modify();
				Asset->CameraEvents.RemoveAt(EventIndex);
				Asset->MarkPackageDirty();
			}
		}
	}
}

void SActionCameraTimelineTrack::RenameEvent(int32 EventIndex, const FVector2D& ScreenPosition)
{
	TSharedRef<STextEntryPopup> TextEntry =
		SNew(STextEntryPopup)
		.Label(FText::FromString(TEXT("Event Name")))
		.OnTextCommitted_Lambda([this, EventIndex](const FText& NewText, ETextCommit::Type CommitType)
			{
				if (CommitType == ETextCommit::OnEnter)
				{
					if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
					{
						if (UActionCameraDirectorAsset* Asset = Toolkit->GetAsset())
						{
							if (Asset->CameraEvents.IsValidIndex(EventIndex))
							{
								Asset->Modify();
								Asset->CameraEvents[EventIndex].EventName = FName(*NewText.ToString());
								Asset->MarkPackageDirty();
							}
						}
					}
				}
				FSlateApplication::Get().DismissAllMenus();
			});

	FSlateApplication::Get().PushMenu(
		SharedThis(this), FWidgetPath(), TextEntry,
		ScreenPosition, FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup));
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
