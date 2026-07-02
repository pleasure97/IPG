// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapon/IPGWeaponFunctionLibrary.h"

FVector UIPGWeaponFunctionLibrary::BezierInterpolate(const FVector& Start, const FVector& Control, const FVector& End, float Alpha)
{
    FVector StartToControlBezierVector = FMath::Lerp(Start, Control, Alpha);
    FVector ControlToEndBezierVector = FMath::Lerp(Control, End, Alpha);
    return FMath::Lerp(StartToControlBezierVector, ControlToEndBezierVector, Alpha);
}
