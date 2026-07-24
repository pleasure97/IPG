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
        // Overwrite with the original array
        OwningAsset->CameraSteps[StepIndex] = StepData;  
        OwningAsset->MarkPackageDirty();
        // Notify the toolkit that the value has changed
        OnChanged.ExecuteIfBound(); 
    }
}
