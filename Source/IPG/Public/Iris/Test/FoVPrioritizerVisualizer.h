// FoVPrioritizerVisualizer.h
#pragma once

#include "Iris/IPGNetPrioritizerDataAsset.h"

#if ENABLE_DRAW_DEBUG

class FFoVPrioritizerVisualizer
{
public:
    // Draw shapes based on a player's current preset.
    static void DrawForConnection(
        const UWorld* World, 
        const FVector& ViewPosition, const FVector& ViewDirection, 
        const FFoVPrioritizerPreset& FoVPrioritizerPreset,
        float Duration = 0.f)  // If it's 0, just one frame
    {
        if (!IsValid(World))
        {
            return;
        }

        // 1. Inner Sphere (Green)
        DrawDebugSphere(World, ViewPosition,
            FoVPrioritizerPreset.InnerSphereRadius, 16,
            FColor::Green, false, Duration, 0, 16.f);

        // 2. Outer Sphere (Yellow)
        DrawDebugSphere(World, ViewPosition,
            FoVPrioritizerPreset.OuterSphereRadius, 32,
            FColor::Yellow, false, Duration, 0, 16.f);

        // 3. FoV Cone
        // Cone Radius = ConeLength * tan(FoV/2)
        const float HalfFoVRad =
            FMath::DegreesToRadians(FoVPrioritizerPreset.ConeFieldOfViewDegrees * 0.5f);
        const float ConeRadius = FoVPrioritizerPreset.ConeLength * FMath::Tan(HalfFoVRad);

        FVector ConePosition = ViewPosition + (ViewDirection * FoVPrioritizerPreset.ConeLength);

        DrawDebugCone(World,
            ConePosition,
            -ViewDirection,
            FoVPrioritizerPreset.ConeLength,
            HalfFoVRad, HalfFoVRad,  // X, Y half angle
            128,
            FColor::Blue, false, Duration, 0, 16.f);

        // Inner Cone (Cyan)
        const float InnerConeRadius =
            FoVPrioritizerPreset.InnerConeLength * FMath::Tan(HalfFoVRad);
        const FVector InnerConeEnd = ViewPosition + ViewDirection * FoVPrioritizerPreset.InnerConeLength;
        DrawDebugCircle(World,
            InnerConeEnd,
            InnerConeRadius,
            32,
            FColor::Cyan, false, Duration, 0, 16.f,
            FVector(0, 1, 0), FVector(0, 0, 1));

        // 4. Line of Sight (Red)
        /*const float LoSHalfLength = FoVPrioritizerPreset.ConeLength * 0.5f;
        const FVector LoSCenter = ViewPosition + ViewDirection * LoSHalfLength;
        DrawDebugCapsule(World,
            LoSCenter,
            LoSHalfLength,
            FoVPrioritizerPreset.LineOfSightWidth * 0.5f,
            FRotationMatrix::MakeFromX(ViewDirection).ToQuat(),
            FColor::Red, false, Duration, 0, 16.f);*/
    }
};
#endif // ENABLE_DRAW_DEBUG