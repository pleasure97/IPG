// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct INNEDataRecord
{
    virtual FString GetHeader() const = 0;
    virtual FString Serialize() const = 0;
    virtual FString GetFileName() const = 0;
    virtual ~INNEDataRecord() = default;
};
