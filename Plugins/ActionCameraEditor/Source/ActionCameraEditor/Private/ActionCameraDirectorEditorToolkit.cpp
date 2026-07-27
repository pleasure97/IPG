// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionCameraDirectorEditorToolkit.h"
#include "ActionCameraDirectorViewport.h"
#include "SActionCameraTimelineTrack.h"

#define LOCTEXT_NAMESPACE "ActionCameraEditor"

const FName FActionCameraDirectorEditorToolkit::ViewportTabId(TEXT("CameraDirectorEditor_Viewport"));
const FName FActionCameraDirectorEditorToolkit::DetailsTabId(TEXT("CameraDirectorEditor_Details"));
const FName FActionCameraDirectorEditorToolkit::TimelineTabId(TEXT("CameraDirectorEditor_Timeline"));

void FActionCameraDirectorEditorToolkit::InitCameraDirector(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InToolkitHost, UActionCameraDirectorAsset* InAsset)
{
	EditingAsset = InAsset;
	// Create Details Panel 
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	// Asset Details
	FDetailsViewArgs AssetDetailsArgs;
	AssetDetailsArgs.bAllowSearch = false;
	AssetDetailsView = PropertyModule.CreateDetailView(AssetDetailsArgs);
	AssetDetailsView->SetObject(EditingAsset);

	AssetDetailsView->OnFinishedChangingProperties()
		.AddSP(this, &FActionCameraDirectorEditorToolkit::OnAssetPropertyChanged);

	// Step Details
	FDetailsViewArgs StepDetailsArgs;
	StepDetailsArgs.bAllowSearch = false;
	StepDetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	// Step Structure Details
	FStructureDetailsViewArgs StructureViewArgs;
	StructureViewArgs.bShowObjects = true;
	StructureViewArgs.bShowAssets = true;
	StructureViewArgs.bShowClasses = true;
	StructureViewArgs.bShowInterfaces = true;

	DetailsView = DetailsView = PropertyModule.CreateDetailView(StepDetailsArgs);

	ViewportWidget = SNew(SActionCameraDirectorViewport).Toolkit(SharedThis(this));
	TimelineWidget = SNew(SActionCameraTimelineTrack).Toolkit(SharedThis(this));

	// Define the docking layout (Left Upper : Viewport / Right : Details / Lower : Timeline)
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("CameraDirectorEditor_Layout_v2")
		->AddArea(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.7f)
				->Split(
					FTabManager::NewStack()
					->AddTab(ViewportTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.7f))
				->Split(
					FTabManager::NewStack()
					->AddTab(DetailsTabId, ETabState::OpenedTab)
					->SetSizeCoefficient(0.3f))
			)
			->Split(FTabManager::NewStack()->AddTab(TimelineTabId, ETabState::OpenedTab)->SetSizeCoefficient(0.3f))
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreatedDefaultToolbar = true;

	InitAssetEditor(Mode, InToolkitHost, FName("ActionCameraDirectorEditorApp"), Layout, bCreateDefaultStandaloneMenu, bCreatedDefaultToolbar, InAsset);

	SetSelectedStep(EditingAsset->CameraSteps.Num() > 0 ? 0 : INDEX_NONE);
}

FName FActionCameraDirectorEditorToolkit::GetToolkitFName() const
{
	return FName("ActionCameraDirectorEditor");
}

FText FActionCameraDirectorEditorToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("ActionCameraEditor", "AppLabel", "Camera Director Editor");
}

FString FActionCameraDirectorEditorToolkit::GetWorldCentricTabPrefix() const
{
	return TEXT("ActionCameraDirector");
}

FLinearColor FActionCameraDirectorEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.2f, 0.6f, 0.8f);
}

void FActionCameraDirectorEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// Register "Vieport Tab" Spawner
	InTabManager->RegisterTabSpawner(ViewportTabId,
		FOnSpawnTab::CreateSP(this, &FActionCameraDirectorEditorToolkit::SpawnTab_Viewport))
		.SetDisplayName(NSLOCTEXT("ActionCameraEditor", "ViewportTab", "Viewport"));

	// Register "Details Tab" Spawner
	InTabManager->RegisterTabSpawner(DetailsTabId,
		FOnSpawnTab::CreateSP(this, &FActionCameraDirectorEditorToolkit::SpawnTab_Details))
		.SetDisplayName(NSLOCTEXT("ActionCameraEditor", "DetailsTab", "Step Detail"));

	// Register "Timeline Tab" Spawner
	InTabManager->RegisterTabSpawner(TimelineTabId,
		FOnSpawnTab::CreateSP(this, &FActionCameraDirectorEditorToolkit::SpawnTab_Timeline))
		.SetDisplayName(NSLOCTEXT("ActionCameraEditor", "TimelineTab", "Tracks"));
}

void FActionCameraDirectorEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager); 

	InTabManager->UnregisterTabSpawner(ViewportTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
	InTabManager->UnregisterTabSpawner(TimelineTabId);
}

void FActionCameraDirectorEditorToolkit::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EditingAsset);
	Collector.AddReferencedObject(SelectedEventProxy);
}

FString FActionCameraDirectorEditorToolkit::GetReferencerName() const
{
	return TEXT("FActionCameraDirectorEditorToolkit");
}

void FActionCameraDirectorEditorToolkit::RecalculateClipTimes()
{
	float AccumulatedTime = 0.f;
	for (FActionCameraClip& ActionCameraClip : EditingAsset->CameraSteps)
	{
		ActionCameraClip.StartTime = AccumulatedTime;

		if (UAnimSequenceBase* AnimSequenceBase = ActionCameraClip.Animation.LoadSynchronous())
		{
			ActionCameraClip.Duration = AnimSequenceBase->GetPlayLength();
		}
		AccumulatedTime += ActionCameraClip.Duration;
	}
}

void FActionCameraDirectorEditorToolkit::SetSelectedStep(int32 StepIndex)
{
	SelectedStepIndex = StepIndex;

	if (EditingAsset && EditingAsset->CameraSteps.IsValidIndex(StepIndex))
	{
		SelectedEventProxy = NewObject<UActionCameraEventProxy>(GetTransientPackage());
		SelectedEventProxy->Init(
			EditingAsset,
			StepIndex,
			FOnActionCameraEventChanged::CreateSP(this, &FActionCameraDirectorEditorToolkit::OnStepProxyChanged));

		DetailsView->SetObject(SelectedEventProxy);
	}
	else
	{
		SelectedEventProxy = nullptr;
		DetailsView->SetObject(nullptr);
	}

	if (ViewportWidget.IsValid())
	{
		ViewportWidget->RefreshCameraForSelectedStep();
	}
}

int32 FActionCameraDirectorEditorToolkit::GetSelectedStepIndex() const
{
	return SelectedStepIndex;
}

UActionCameraDirectorAsset* FActionCameraDirectorEditorToolkit::GetAsset() const
{
	return EditingAsset;
}

TSharedPtr<SActionCameraDirectorViewport> FActionCameraDirectorEditorToolkit::GetViewportWidget() const
{
	return ViewportWidget;
}

void FActionCameraDirectorEditorToolkit::OnAssetPropertyChanged(const FPropertyChangedEvent& Event)
{
	RecalculateClipTimes();

	SetSelectedStep(
		EditingAsset->CameraSteps.IsValidIndex(SelectedStepIndex) ? SelectedStepIndex : (EditingAsset->CameraSteps.Num() > 0 ? 0 : INDEX_NONE));

	if (ViewportWidget.IsValid())
	{
		ViewportWidget->RefreshCameraForSelectedStep();
	}

	if (TimelineWidget.IsValid())
	{
		TimelineWidget->Invalidate(EInvalidateWidgetReason::Layout);
	}
}

void FActionCameraDirectorEditorToolkit::OnStepProxyChanged()
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->RefreshCameraForSelectedStep();
	}
}

TSharedRef<SDockTab> FActionCameraDirectorEditorToolkit::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(NSLOCTEXT("ActionCameraEditor", "ViewportTab", "Viewport"))[ViewportWidget.ToSharedRef()];
}

TSharedRef<SDockTab> FActionCameraDirectorEditorToolkit::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(NSLOCTEXT("ActionCameraEditor", "DetailsTab", "Details"))
		[
			SNew(SSplitter)
				.Orientation(Orient_Vertical)

				+ SSplitter::Slot().Value(0.65f)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 4.0f, 4.0f, 0.0f)
						[
							SNew(STextBlock)
								.Text(NSLOCTEXT("ActionCameraEditor", "AssetHeader", "Asset"))
								.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							AssetDetailsView.ToSharedRef()
						]
				]

			+ SSplitter::Slot().Value(0.35f)
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot().AutoHeight().Padding(4.0f, 4.0f, 4.0f, 0.0f)
						[
							SNew(STextBlock)
								.Text(NSLOCTEXT("ActionCameraEditor", "StepHeader", "Selected Step"))
								.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SVerticalBox::Slot().FillHeight(1.0f)
						[
							DetailsView.ToSharedRef()
						]
				]
		];
}

TSharedRef<SDockTab> FActionCameraDirectorEditorToolkit::SpawnTab_Timeline(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab).Label(NSLOCTEXT("ActionCameraEditor", "TimelineTab", "Tracks"))
	[
		SNew(SScrollBox).Orientation(Orient_Horizontal)
			+ SScrollBox::Slot()[TimelineWidget.ToSharedRef()]
	];
}

#undef LOCTEXT_NAMESPACE // ActionCameraEditor