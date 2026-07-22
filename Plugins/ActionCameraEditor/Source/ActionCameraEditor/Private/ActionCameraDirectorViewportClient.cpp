// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionCameraDirectorViewportClient.h"
#include "ActionCameraDirectorEditorToolkit.h"
#include "ActionCameraDirectorViewport.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "Core/CameraAsset.h"
#include "Directors/SingleCameraDirector.h"
#include "Build/CameraBuildLog.h"
#include "GameFramework/GameplayCameraComponent.h"
#include "CineCameraComponent.h"

FActionCameraDirectorViewportClient::FActionCameraDirectorViewportClient(
	TWeakPtr<FActionCameraDirectorEditorToolkit> InToolkit,
	FPreviewScene* InPreviewScene, 
	const TSharedRef<SActionCameraDirectorViewport>& InViewportWidget)
	: FEditorViewportClient(nullptr, InPreviewScene, InViewportWidget), ToolkitPtr(InToolkit)
{
	SetRealtime(true);

    AActor* PreviewActor = InPreviewScene->GetWorld()->SpawnActor<AActor>(); 
	
    // Spawn character for testing action camera system
    if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
    {
        if (USkeletalMesh* PreviewSkeletalMesh = Toolkit->GetAsset()->PreviewMesh.LoadSynchronous())
        {
            PreviewMeshComponent = NewObject<UDebugSkelMeshComponent>();
            PreviewMeshComponent->SetSkeletalMesh(PreviewSkeletalMesh);
            PreviewMeshComponent->RegisterComponentWithWorld(InPreviewScene->GetWorld());
        }
    }

    // Create transient director and camera asset
    PreviewDirector = NewObject<USingleCameraDirector>(); 
    PreviewCameraAsset = NewObject<UCameraAsset>(); 
    PreviewCameraAsset->SetCameraDirector(PreviewDirector);

    // Since these are unsaved, transient assets, they do not go through the PreSave path and require a manual build.
    UE::Cameras::FCameraBuildLog BuildLog;
    PreviewCameraAsset->BuildCamera(BuildLog);

    PreviewCameraComponent = NewObject<UGameplayCameraComponent>(PreviewActor);
    PreviewCameraComponent->CameraReference.SetCameraAsset(PreviewCameraAsset);
    PreviewCameraComponent->SetupAttachment(PreviewMeshComponent);
    PreviewCameraComponent->RegisterComponentWithWorld(InPreviewScene->GetWorld());

    PreviewCameraComponent->ActivateCameraForPlayerController(
        nullptr, false, EGameplayCameraComponentActivationMode::Push);

    RefreshCameraForSelectedStep();
}

void FActionCameraDirectorViewportClient::Tick(float DeltaSeconds)
{
    FEditorViewportClient::Tick(DeltaSeconds);

    if (PreviewCameraComponent)
    {
        PreviewCameraComponent->TickComponent(DeltaSeconds, LEVELTICK_All, nullptr);

        if (UCineCameraComponent* OutputCamera = PreviewCameraComponent->GetOutputCameraComponent())
        {
            SetViewLocation(OutputCamera->GetComponentLocation());
            SetViewRotation(OutputCamera->GetComponentRotation());
            ViewFOV = OutputCamera->FieldOfView;
        }
    }
}

void FActionCameraDirectorViewportClient::RefreshCameraForSelectedStep()
{
    TSharedPtr<FActionCameraDirectorEditorToolkit> ToolKit = ToolkitPtr.Pin();
    if (!ToolKit.IsValid())
    {
        return;
    }

    UActionCameraDirectorAsset* ActionCameraDirectorAsset = ToolKit->GetAsset(); 
    int32 SelectedIndex = ToolKit->GetSelectedStepIndex(); 

    if (ActionCameraDirectorAsset && ActionCameraDirectorAsset->CameraSteps.IsValidIndex(SelectedIndex) && PreviewDirector)
    {
        const FActionCameraStep& ActionCameraStep = ActionCameraDirectorAsset->CameraSteps[SelectedIndex];
        PreviewDirector->CameraRig = ActionCameraStep.CameraRig.LoadSynchronous();
    }
}

/* Playing */
bool FActionCameraDirectorViewportClient::IsPlaying() const
{
    return bPlaying;
}

void FActionCameraDirectorViewportClient::SetPlaying(bool bNewPlaying)
{
    bPlaying = bNewPlaying;
}

bool FActionCameraDirectorViewportClient::IsLooping() const
{
    return bLooping;
}

void FActionCameraDirectorViewportClient::SetLooping(bool bNewLooping)
{
    bLooping = bNewLooping;
}

float FActionCameraDirectorViewportClient::GetPlaybackTime() const
{
    return PlaybackTime;
}

void FActionCameraDirectorViewportClient::SetPlaybackTime(float NewPlaybackTime)
{
    PlaybackTime = NewPlaybackTime;
}

float FActionCameraDirectorViewportClient::GetMaxPlaybackTime() const
{
    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid())
    {
        return 0.0f;
    }

    if (UAnimSequence* Anim = Toolkit->GetAsset()->ReferenceAnimation.LoadSynchronous())
    {
        return Anim->GetPlayLength();
    }

    return 0.f;
}

void FActionCameraDirectorViewportClient::SetMaxPlaybackTime(float NewMaxPlaybackTime)
{
    MaxPlaybackTime = NewMaxPlaybackTime;
}
