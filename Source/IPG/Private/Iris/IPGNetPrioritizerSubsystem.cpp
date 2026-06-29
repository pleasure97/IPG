// Fill out your copyright notice in the Description page of Project Settings.


#include "Iris/IPGNetPrioritizerSubsystem.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/Prioritization/FieldOfViewNetObjectPrioritizer.h"
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"   
#include "Iris/ReplicationSystem/ReplicationSystem.h"     
#endif
#include "Iris/DynamicFoVNetObjectPrioritizer.h"
#include "Iris/Test/FoVPrioritizerVisualizer.h"

static FAutoConsoleCommandWithWorld GFoVDebugDrawCmd(
    TEXT("Iris.FoV.DebugDraw"),
    TEXT("Toggle FoV Prioritizer debug visualization"),
    FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
        {
            if (UIPGNetPrioritizerSubsystem* IPGNetPrioritizerSubsystem = World->GetSubsystem<UIPGNetPrioritizerSubsystem>())
            {
                IPGNetPrioritizerSubsystem->SetDebugDraw(!IPGNetPrioritizerSubsystem->GetDebugDraw());
            }
        })
);

void UIPGNetPrioritizerSubsystem::RegisterNetPrioritizerPreset(UIPGFoVNetPrioritizerDataAsset* NetPrioritizerPreset)
{
    if (NetPrioritizerPreset)
    {
        MapPrioritizerMappings.Add(NetPrioritizerPreset->MapType, NetPrioritizerPreset);
    }
}

void UIPGNetPrioritizerSubsystem::ApplyNetPrioritizerPreset(EIPGMapType MapType, float BlendTime)
{
    UIPGFoVNetPrioritizerDataAsset** FoundNetPrioritizerPreset = MapPrioritizerMappings.Find(MapType);
    if (!FoundNetPrioritizerPreset || !(*FoundNetPrioritizerPreset))
    {
        return;
    }

    const FFoVPrioritizerPreset& PresetData = (*FoundNetPrioritizerPreset)->FoVPrioritizerPresetData;
    BlendDuration = FMath::Max(BlendTime, 0.01f);

    if (!bHasBeenInitialized)
    {
        CurrentData = PresetData;
        TargetData = PresetData;
        BlendAlpha = 1.0f;
        bHasBeenInitialized = true;
        ApplyToConfig(PresetData);

        return;
    }

    TargetData = PresetData;
    BlendAlpha = 0.0f;
}

void UIPGNetPrioritizerSubsystem::ApplyNetPrioritizerPresetForConnection(uint32 ConnectionId, EIPGMapType MapType)
{
    UIPGFoVNetPrioritizerDataAsset** FoundNetPrioritizerPreset = MapPrioritizerMappings.Find(MapType);
    if (!FoundNetPrioritizerPreset || !(*FoundNetPrioritizerPreset))
    {
        return;
    }

    if (UDynamicFoVNetObjectPrioritizer* DynamicFovNetPrioritizer = GetDynamicFoVNetObjectPrioritizer())
    {
        UE_LOG(LogTemp, Log, TEXT("SetConnection - Prioritizer ptr = %p, ConnectionId = %u"), DynamicFovNetPrioritizer, ConnectionId);
        DynamicFovNetPrioritizer->SetConnectionPreset(ConnectionId, (*FoundNetPrioritizerPreset)->FoVPrioritizerPresetData);
    }
}

void UIPGNetPrioritizerSubsystem::ClearConnectionPreset(uint32 ConnectionId)
{
    if (UDynamicFoVNetObjectPrioritizer* DynamicFovNetPrioritizer = GetDynamicFoVNetObjectPrioritizer())
    {
        DynamicFovNetPrioritizer->ClearConnectionPreset(ConnectionId);
    }
}

void UIPGNetPrioritizerSubsystem::SetDebugDraw(bool bEnabled)
{
    bDebugDrawEnabled = bEnabled;
}

bool UIPGNetPrioritizerSubsystem::GetDebugDraw() const
{
    return bDebugDrawEnabled;
}

bool UIPGNetPrioritizerSubsystem::ShouldCreateSubsystem(UObject* Object) const
{
    return true; // GetWorld() && GetWorld()->GetNetMode() != NM_Client;
}

void UIPGNetPrioritizerSubsystem::Tick(float DeltaTime)
{
#if ENABLE_DRAW_DEBUG
    if (bDebugDrawEnabled)
    {
        DebugDraw();
    }
#endif

    if (BlendAlpha >= 1.0f)
    {
        return;
    }

    BlendAlpha = FMath::Clamp(BlendAlpha + DeltaTime / BlendDuration, 0.0f, 1.0f);

    FFoVPrioritizerPreset Blended;
    Blended.InnerSphereRadius = FMath::Lerp(CurrentData.InnerSphereRadius, TargetData.InnerSphereRadius, BlendAlpha);
    Blended.OuterSphereRadius = FMath::Lerp(CurrentData.OuterSphereRadius, TargetData.OuterSphereRadius, BlendAlpha);
    Blended.InnerSpherePriority = FMath::Lerp(CurrentData.InnerSpherePriority, TargetData.InnerSpherePriority, BlendAlpha);
    Blended.OuterSpherePriority = FMath::Lerp(CurrentData.OuterSpherePriority, TargetData.OuterSpherePriority, BlendAlpha);
    Blended.OutsidePriority = FMath::Lerp(CurrentData.OutsidePriority, TargetData.OutsidePriority, BlendAlpha);
    Blended.ConeFieldOfViewDegrees = FMath::Lerp(CurrentData.ConeFieldOfViewDegrees, TargetData.ConeFieldOfViewDegrees, BlendAlpha);
    Blended.ConeLength = FMath::Lerp(CurrentData.ConeLength, TargetData.ConeLength, BlendAlpha);
    Blended.InnerConeLength = FMath::Lerp(CurrentData.InnerConeLength, TargetData.InnerConeLength, BlendAlpha);
    Blended.MaxConePriority = FMath::Lerp(CurrentData.MaxConePriority, TargetData.MaxConePriority, BlendAlpha);
    Blended.MinConePriority = FMath::Lerp(CurrentData.MinConePriority, TargetData.MinConePriority, BlendAlpha);
    Blended.LineOfSightWidth = FMath::Lerp(CurrentData.LineOfSightWidth, TargetData.LineOfSightWidth, BlendAlpha);
    Blended.LineOfSightPriority = FMath::Lerp(CurrentData.LineOfSightPriority, TargetData.LineOfSightPriority, BlendAlpha);

    ApplyToConfig(Blended);

    if (BlendAlpha >= 1.0f)
    {
        CurrentData = TargetData;
    }
}

TStatId UIPGNetPrioritizerSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UIPGNetPrioritizerSubsystem, STATGROUP_Tickables);
}

UDynamicFoVNetObjectPrioritizer* UIPGNetPrioritizerSubsystem::GetDynamicFoVNetObjectPrioritizer() const
{
#if UE_WITH_IRIS
    // Get replication system
    UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(GetWorld());
    if (!ReplicationSystem)
    {
        return nullptr;
    }

    return Cast<UDynamicFoVNetObjectPrioritizer>(ReplicationSystem->GetPrioritizer(FName("FoVPrioritizer")));
#else
    return nullptr;
#endif
}

void UIPGNetPrioritizerSubsystem::ApplyToConfig(const FFoVPrioritizerPreset& Data)
{
    if (UDynamicFoVNetObjectPrioritizer* Prioritizer = GetDynamicFoVNetObjectPrioritizer())
    {
        Prioritizer->SetGlobalPreset(Data);
    }
}

void UIPGNetPrioritizerSubsystem::DebugDraw()
{
#if ENABLE_DRAW_DEBUG
    UDynamicFoVNetObjectPrioritizer* DynamicFoVNetObjectPrioritizer = GetDynamicFoVNetObjectPrioritizer();
    if (!IsValid(DynamicFoVNetObjectPrioritizer))
    {
        return;
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        // Get player controller
        const APlayerController* PC = It->Get();
        if (!IsValid(PC) || !IsValid(PC->GetNetConnection()))
        {
            continue;
        }

        // Get pawn
        const APawn* Pawn = PC->GetPawn();
        if (!IsValid(Pawn))
        {
            continue;
        }

        const uint32 ConnectionId = PC->GetNetConnection()->GetConnectionHandle().GetParentConnectionId();

        const FFoVPrioritizerPreset* Preset = DynamicFoVNetObjectPrioritizer->GetConnectionPreset(ConnectionId);

        const FFoVPrioritizerPreset& DrawPreset = Preset ? *Preset : DynamicFoVNetObjectPrioritizer->GetGlobalPreset();

        // Get player view point's position and rotation for visualization
        FVector ViewPosition;
        FRotator ViewRotation;
        PC->GetPlayerViewPoint(ViewPosition, ViewRotation);
        const FVector ViewDirection = ViewRotation.Vector();

        FFoVPrioritizerVisualizer::DrawForConnection(GetWorld(), ViewPosition, ViewDirection, DrawPreset);
    }
#endif
}
