// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ActionCameraDirectorViewportClient.h"
#include "ActionCameraDirectorEditorToolkit.h"
#include "ActionCameraDirectorViewport.h"
#include "ActionCameraPreviewCharacter.h"
#include "Animation/DebugSkelMeshComponent.h"
#include "GameFramework/GameplayCameraRigComponent.h"
#include "CineCameraComponent.h"
#include "AnimPreviewInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequenceHelpers.h"

FActionCameraDirectorViewportClient::FActionCameraDirectorViewportClient(
	TWeakPtr<FActionCameraDirectorEditorToolkit> InToolkit,
	FPreviewScene* InPreviewScene, 
	const TSharedRef<SActionCameraDirectorViewport>& InViewportWidget)
	: FEditorViewportClient(nullptr, InPreviewScene, InViewportWidget), ToolkitPtr(InToolkit)
{
	SetRealtime(true);

    PreviewMeshTickFunction.bCanEverTick = true;
    PreviewMeshTickFunction.Target = nullptr;

    PreviewCameraTickFunction.bCanEverTick = true;
    PreviewCameraTickFunction.Target = nullptr;

    PreviewCharacter = InPreviewScene->GetWorld()->SpawnActor<AActionCameraPreviewCharacter>();
    if (PreviewCharacter)
    {
        PreviewCharacter->SetActorRotation(FRotator::ZeroRotator);
        PreviewCharacter->GetCapsuleComponent()->SetVisibility(false);
        PreviewCharacter->GetCapsuleComponent()->SetHiddenInGame(true);

        if (UCharacterMovementComponent* CharacterMovementComponent = PreviewCharacter->GetCharacterMovement())
        {
            CharacterMovementComponent->GravityScale = 0.f;
            CharacterMovementComponent->SetMovementMode(MOVE_Flying);
        }
    }
    
    // Spawn character for testing action camera system
    if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
    {
        if (UActionCameraDirectorAsset* ActionCameraDirectorAsset = Toolkit->GetAsset())
        {
            if (USkeletalMesh* PreviewSkeletalMesh = ActionCameraDirectorAsset->PreviewMesh.LoadSynchronous())
            {
                PreviewMeshComponent = CastChecked<UDebugSkelMeshComponent>(PreviewCharacter->GetMesh());
                PreviewMeshComponent->SetSkeletalMesh(PreviewSkeletalMesh);
                PreviewMeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
            }
        }
    }

    PreviewCameraComponent = NewObject<UGameplayCameraRigComponent>(PreviewCharacter);
    PreviewCameraComponent->SetupAttachment(PreviewCharacter->GetCapsuleComponent());
    PreviewCameraComponent->RegisterComponentWithWorld(InPreviewScene->GetWorld());

    DummyController = InPreviewScene->GetWorld()->SpawnActor<APlayerController>();
    if (DummyController)
    {
        DummyController->Possess(PreviewCharacter);
    }

    PreviewCameraComponent->ActivateCameraForPlayerController(
        DummyController, false, EGameplayCameraComponentActivationMode::Push);

    RefreshCameraForSelectedStep();
}

void FActionCameraDirectorViewportClient::Tick(float DeltaSeconds)
{
    FEditorViewportClient::Tick(DeltaSeconds);

    const float PrevTime = PlaybackTime;

    if (bPlaying)
    {
        float NewTime = PlaybackTime + DeltaSeconds;
        const float MaxTime = GetMaxPlaybackTime();

        if (MaxTime > KINDA_SMALL_NUMBER && NewTime > MaxTime)
        {
            NewTime = bLooping ? FMath::Fmod(NewTime, MaxTime) : MaxTime;
            if (!bLooping)
            {
                bPlaying = false;   
            }
        }

        PlaybackTime = NewTime;
    }

    UpdatePreviewAnimation(PlaybackTime);

    if (PreviewScene)
    {
        PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
    }

    if (TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin())
    {
        if (UAnimSequence* AnimSequence = Toolkit->GetAsset()->ReferenceAnimation.LoadSynchronous())
        {
            if (PreviewCharacter && AnimSequence->HasRootMotion() && !FMath::IsNearlyEqual(PrevTime, PlaybackTime))
            {
                const FTransform RootMotionDelta =
                    UE::Anim::ExtractRootMotionFromAnimationAsset(AnimSequence, nullptr, PrevTime, PlaybackTime);
                PreviewCharacter->AddActorLocalTransform(RootMotionDelta);
            }
        }
    }

    if (PreviewCameraComponent)
    {
        PreviewCameraComponent->TickComponent(DeltaSeconds, LEVELTICK_All, &PreviewCameraTickFunction);

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
    if (!ToolKit.IsValid() || !IsValid(PreviewCameraComponent))
    {
        return;
    }

    UActionCameraDirectorAsset* ActionCameraDirectorAsset = ToolKit->GetAsset(); 
    int32 SelectedIndex = ToolKit->GetSelectedStepIndex(); 

    UCameraRigAsset* TargetCameraRig = nullptr;
    if (ActionCameraDirectorAsset && ActionCameraDirectorAsset->CameraSteps.IsValidIndex(SelectedIndex))
    {
        TargetCameraRig = ActionCameraDirectorAsset->CameraSteps[SelectedIndex].CameraRig.LoadSynchronous();
    }

    if (PreviewCameraComponent->CameraRigReference.GetCameraRig() == TargetCameraRig)
    {
        return;
    }

    PreviewCameraComponent->CameraRigReference.SetCameraRig(TargetCameraRig);

    FStructProperty* CameraRigStructProperty = FindFProperty<FStructProperty>(
        UGameplayCameraRigComponent::StaticClass(),
        GET_MEMBER_NAME_CHECKED(UGameplayCameraRigComponent, CameraRigReference));

    FProperty* CameraRigProperty = CameraRigStructProperty ? FindFProperty<FProperty>(CameraRigStructProperty->Struct, TEXT("CameraRig")) : nullptr;
    if (CameraRigProperty)
    {
        FPropertyChangedEvent PropertyChangedEvent(CameraRigProperty);
        PropertyChangedEvent.SetActiveMemberProperty(CameraRigStructProperty);
        PreviewCameraComponent->PostEditChangeProperty(PropertyChangedEvent);
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

void FActionCameraDirectorViewportClient::UpdatePreviewAnimation(float Time)
{
    if (!PreviewMeshComponent)
    {
        return;
    }

    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid() || !Toolkit->GetAsset())
    {
        return;
    }

    UAnimSequence* AnimSequence = Toolkit->GetAsset()->ReferenceAnimation.LoadSynchronous();
    if (!IsValid(AnimSequence))
    {
        return;
    }
    
    if (PreviewMeshComponent->PreviewInstance == nullptr || PreviewMeshComponent->PreviewInstance->GetCurrentAsset() != AnimSequence)
    {
        PreviewMeshComponent->EnablePreview(true, AnimSequence);
        PreviewMeshComponent->SetProcessRootMotionMode(EProcessRootMotionMode::LoopAndReset);
    }

    if (UAnimPreviewInstance* AnimPreviewInstance = PreviewMeshComponent->PreviewInstance)
    {
        AnimPreviewInstance->SetPosition(Time, false);
    }

    PreviewMeshComponent->TickComponent(0.f, LEVELTICK_All, &PreviewMeshTickFunction);
}
