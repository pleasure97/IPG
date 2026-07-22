// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraStepProxy.generated.h"

DECLARE_DELEGATE(FOnActionCameraStepChanged);

UCLASS()
class UActionCameraStepProxy : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Step")
    FActionCameraStep StepData;

    void Init(UActionCameraDirectorAsset* InOwningAsset, int32 InStepIndex, FOnActionCameraStepChanged InOnChanged);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
    UPROPERTY()
    TObjectPtr<UActionCameraDirectorAsset> OwningAsset = nullptr;

    int32 StepIndex = INDEX_NONE;

    FOnActionCameraStepChanged OnChanged;
};