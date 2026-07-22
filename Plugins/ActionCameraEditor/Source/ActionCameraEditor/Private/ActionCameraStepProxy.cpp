// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionCameraStepProxy.h"

void UActionCameraStepProxy::Init(UActionCameraDirectorAsset* InOwningAsset, int32 InStepIndex, FOnActionCameraStepChanged InOnChanged)
{
    OwningAsset = InOwningAsset;
    StepIndex = InStepIndex;
    OnChanged = InOnChanged;

    if (OwningAsset && OwningAsset->CameraSteps.IsValidIndex(StepIndex))
    {
        StepData = OwningAsset->CameraSteps[StepIndex];
    }
}

void UActionCameraStepProxy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (OwningAsset && OwningAsset->CameraSteps.IsValidIndex(StepIndex))
    {
        OwningAsset->Modify();
        OwningAsset->CameraSteps[StepIndex] = StepData;   // 원본 배열로 되쓰기
        OwningAsset->MarkPackageDirty();

        OnChanged.ExecuteIfBound();  // Toolkit에 "값이 바뀌었다" 통지
    }
}
