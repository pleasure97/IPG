// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraEventProxy.generated.h"

DECLARE_DELEGATE(FOnActionCameraEventChanged);

UCLASS()
class UActionCameraEventProxy : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Step")
    FActionCameraEvent EventData;

    void Init(UActionCameraDirectorAsset* InOwningAsset, int32 InIndex, FOnActionCameraEventChanged InOnChanged);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
    UPROPERTY()
    TObjectPtr<UActionCameraDirectorAsset> OwningAsset = nullptr;

    int32 Index = INDEX_NONE;

    FOnActionCameraEventChanged OnChanged;
};