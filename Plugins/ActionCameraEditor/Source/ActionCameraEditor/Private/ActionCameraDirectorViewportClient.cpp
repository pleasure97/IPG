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
#include "Components/ArrowComponent.h"
#include "Animation/AnimSequenceHelpers.h"
#include "Core/CameraVariableCollection.h"
#include "Kismet/GameplayStatics.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

static UCameraVariableAsset* FindVariableInCollectionByName(UCameraVariableCollection* Collection, FName VariableName)
{
    if (!IsValid(Collection))
    {
        return nullptr;
    }

    for (UCameraVariableAsset* Variable : Collection->Variables)
    {
        if (Variable && Variable->GetFName() == VariableName)
        {
            return Variable;
        }
#if WITH_EDITORONLY_DATA
        if (Variable && FName(*Variable->GetDisplayName()) == VariableName)
        {
            return Variable;
        }
#endif
    }
    return nullptr;
}

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
        PreviewCharacter->GetArrowComponent()->SetVisibility(false);
        PreviewCharacter->GetArrowComponent()->SetHiddenInGame(true);
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

    RefreshCameraForTime(0.f);
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
    RefreshCameraForTime(PlaybackTime);

    if (PreviewScene)
    {
        PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
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

void FActionCameraDirectorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
    FEditorViewportClient::Draw(View, PDI);

    if (!IsValid(PreviewCameraComponent))
    {
        return;
    }

    TSharedPtr<UE::Cameras::FCameraEvaluationContext> Context = PreviewCameraComponent->GetEvaluationContext();
    if (!Context.IsValid())
    {
        return;
    }

    const UE::Cameras::FCameraVariableTable& VariableTable = Context->GetInitialResult().VariableTable;

    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid())
    {
        return;
    }

    for (const FActionCameraEvent& Event : Toolkit->GetAsset()->CameraEvents)
    {
        UCameraVariableCollection* Collection = Event.VariableCollection.LoadSynchronous();
        if (!IsValid(Collection))
        {
            continue;
        }

        for (UCameraVariableAsset* Variable : Collection->Variables)
        {
            if (UVector3dCameraVariable* VecVar = Cast<UVector3dCameraVariable>(Variable))
            {
                FVector3d Value;
                if (VariableTable.TryGetValue<FVector3d>(VecVar->GetVariableID(), Value))
                {
                    // Wire sphere which represents the variable's location
                    DrawWireSphere(PDI, FVector(Value), FLinearColor::Green, 15.0f, 12, SDPG_World);

                    // Draw the direction from original to the variable's location
                    PDI->DrawLine(FVector::ZeroVector, FVector(Value), FLinearColor::Green, SDPG_World, 1.0f);
                }
            }
        }
    }
}

void FActionCameraDirectorViewportClient::RefreshCameraForTime(float Time)
{
    UCameraRigAsset* NewCameraRig = ResolveActiveCameraRig(Time);

    if (LastAppliedRig != NewCameraRig)
    {
        LastAppliedRig = NewCameraRig;
        PreviewCameraComponent->CameraRigReference.SetCameraRig(NewCameraRig);

        FStructProperty* MemberProp = FindFProperty<FStructProperty>(
            UGameplayCameraRigComponent::StaticClass(),
            GET_MEMBER_NAME_CHECKED(UGameplayCameraRigComponent, CameraRigReference));
        FProperty* InnerProp = MemberProp ? FindFProperty<FProperty>(MemberProp->Struct, TEXT("CameraRig")) : nullptr;
        if (InnerProp)
        {
            FPropertyChangedEvent PropertyChangedEvent(InnerProp);
            PropertyChangedEvent.SetActiveMemberProperty(MemberProp);
            PreviewCameraComponent->PostEditChangeProperty(PropertyChangedEvent);
        }
    }

    ApplyVariableOverridesForTime(Time);
}

FResolvedClip FActionCameraDirectorViewportClient::ResolveClipAtTime(float GlobalTime) const
{
    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid())
    {
        return FResolvedClip(nullptr, 0.f); 
    }

    const TArray<FActionCameraClip>& ActionCameraClips = Toolkit->GetAsset()->CameraSteps;
    for (int32 i = ActionCameraClips.Num() - 1; i >= 0; --i)
    {
        if (GlobalTime >= ActionCameraClips[i].StartTime)
        {
            return FResolvedClip(ActionCameraClips[i].Animation.LoadSynchronous(), GlobalTime - ActionCameraClips[i].StartTime);
        }
    }

    return FResolvedClip(nullptr, 0.f);
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

/* Looping */
bool FActionCameraDirectorViewportClient::IsLooping() const
{
    return bLooping;
}

void FActionCameraDirectorViewportClient::SetLooping(bool bNewLooping)
{
    bLooping = bNewLooping;
}

/* Playback Time */
float FActionCameraDirectorViewportClient::GetPlaybackTime() const
{
    return PlaybackTime;
}

void FActionCameraDirectorViewportClient::SetPlaybackTime(float NewPlaybackTime)
{
    PlaybackTime = NewPlaybackTime;
}

/* Max Playback Time */
float FActionCameraDirectorViewportClient::GetMaxPlaybackTime() const
{
    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid() || !Toolkit->GetAsset())
    {
        return 0.0f;
    }

    const TArray<FActionCameraClip>& CameraSteps = Toolkit->GetAsset()->CameraSteps;
    if (CameraSteps.Num() == 0)
    {
        return 0.f;
    }

    const FActionCameraClip& LastClip = CameraSteps.Last();
    return LastClip.StartTime + LastClip.Duration;
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

    FResolvedClip ResolvedClip = ResolveClipAtTime(Time); 
    if (!IsValid(ResolvedClip.AnimSequenceBase))
    {
        return;
    }

    if (PreviewMeshComponent->PreviewInstance == nullptr || PreviewMeshComponent->PreviewInstance->GetCurrentAsset() != ResolvedClip.AnimSequenceBase)
    {
        PreviewMeshComponent->EnablePreview(true, ResolvedClip.AnimSequenceBase);
        PreviewMeshComponent->SetProcessRootMotionMode(EProcessRootMotionMode::LoopAndReset);
    }

    if (UAnimPreviewInstance* AnimPreviewInstance = PreviewMeshComponent->PreviewInstance)
    {
        AnimPreviewInstance->SetPosition(ResolvedClip.LocalTime, false);
    }

    PreviewMeshComponent->TickComponent(0.f, LEVELTICK_All, &PreviewMeshTickFunction);
}

UCameraRigAsset* FActionCameraDirectorViewportClient::ResolveActiveCameraRig(float Time) const
{
    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid())
    {
        return nullptr;
    }

    UActionCameraDirectorAsset* Asset = Toolkit->GetAsset(); 
    if (!IsValid(Asset))
    {
        return nullptr;
    }

    for (int32 i = Asset->CameraEvents.Num() - 1; i >= 0; --i)
    {
        const FActionCameraEvent& Event = Asset->CameraEvents[i];
        if (Event.Duration > KINDA_SMALL_NUMBER && Time >= Event.Time && Time < Event.Time + Event.Duration && !Event.CameraRig.IsNull())
        {
            return Event.CameraRig.LoadSynchronous();
        }
    }

    return Asset->DefaultCameraRig.LoadSynchronous();
}

UCameraVariableCollection* FActionCameraDirectorViewportClient::ResolveActiveVariableCollection(float Time) const
{
    return nullptr;
}

void FActionCameraDirectorViewportClient::ApplyVariableOverridesForTime(float Time)
{
    // Get action camera director editor toolkit
    TSharedPtr<FActionCameraDirectorEditorToolkit> Toolkit = ToolkitPtr.Pin();
    if (!Toolkit.IsValid() || !IsValid(PreviewCameraComponent))
    {
        return;
    }

    // Get action camera director asset from toolkit
    UActionCameraDirectorAsset* Asset = Toolkit->GetAsset();
    if (!IsValid(Asset))
    {
        return;
    }

    // Get evaluation context from preview camera component
    TSharedPtr<UE::Cameras::FCameraEvaluationContext> Context = PreviewCameraComponent->GetEvaluationContext();
    if (!Context.IsValid())
    {
        return;
    }

    // Get camera variable table
    UE::Cameras::FCameraVariableTable& VariableTable = Context->GetInitialResult().VariableTable;

    for (int32 i = Asset->CameraEvents.Num() - 1; i >= 0; --i)
    {
        const FActionCameraEvent& ActionCameraEvent = Asset->CameraEvents[i];

        // Duration > 0 
        if (ActionCameraEvent.Duration <= KINDA_SMALL_NUMBER)
        {
            continue;
        }

        // Event.Time <= Time < Event.Time + Event.Duratrion
        if (Time < ActionCameraEvent.Time || Time >= ActionCameraEvent.Time + ActionCameraEvent.Duration)
        {
            continue;
        }

        // Get camera variable collection from event
        UCameraVariableCollection* Collection = ActionCameraEvent.VariableCollection.LoadSynchronous();
        if (!IsValid(Collection))
        {
            continue;
        }

        // Iterate camera collection variables
        for (const TPair<FName, float>& Override : ActionCameraEvent.VariableOverrides)
        {
            // Get camera variable asset from the collection variable
            UCameraVariableAsset* VarAsset = FindVariableInCollectionByName(Collection, Override.Key);
            if (UFloatCameraVariable* FloatVar = Cast<UFloatCameraVariable>(VarAsset))
            {
                VariableTable.TrySetValue<float>(FloatVar->GetVariableID(), Override.Value);
            }
        }
        break;
    }
}
