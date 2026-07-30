// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionCameraEventProxy.h"

void UActionCameraEventProxy::Init(UActionCameraDirectorAsset* InOwningAsset, int32 InIndex, FOnActionCameraEventChanged InOnChanged)
{
    OwningAsset = InOwningAsset;
    Index = InIndex;
    OnChanged = InOnChanged;

    if (OwningAsset && OwningAsset->CameraEvents.IsValidIndex(Index))
    {
        EventData = OwningAsset->CameraEvents[Index];
    }
}

void UActionCameraEventProxy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (OwningAsset && OwningAsset->CameraEvents.IsValidIndex(Index))
    {
        OwningAsset->Modify();
        // Overwrite with the original array
        OwningAsset->CameraEvents[Index] = EventData;  
        OwningAsset->MarkPackageDirty();
        // Notify the toolkit that the value has changed
        OnChanged.ExecuteIfBound(); 
    }
}
