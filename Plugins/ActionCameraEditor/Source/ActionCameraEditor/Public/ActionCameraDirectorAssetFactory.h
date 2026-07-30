// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Factories/Factory.h"
#include "ActionCameraDirectorAssetFactory.generated.h"

UCLASS()
class UActionCameraDirectorAssetFactory : public UFactory
{
    GENERATED_BODY()

public:
    UActionCameraDirectorAssetFactory();

    virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};