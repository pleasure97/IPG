// Fill out your copyright notice in the Description page of Project Settings.


#include "Iris/Test/IrisTestGameMode.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Iris/IPGNetPrioritizerSubsystem.h"
#include "Iris/IPGNetPrioritizerDataAsset.h"

void AIrisTestGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer); 

    if (ACharacter* CharacterPawn = Cast<ACharacter>(NewPlayer->GetPawn()))
    {
        UCapsuleComponent* CharacterCapsule = CharacterPawn->GetCapsuleComponent();
        if (!IsValid(CharacterCapsule))
        {
            return;
        }

        // Prevent collisions between other characters and other cameras
        CharacterCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        CharacterCapsule->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        // Prevent collision betweeen self character and camera
        if (USkeletalMeshComponent* CharacterMesh = CharacterPawn->GetMesh())
        {
            CharacterMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        }
    }

    // Get net prioritizer subsystem and apply default prioritizer preset
    UIPGNetPrioritizerSubsystem* Subsystem = GetWorld()->GetSubsystem<UIPGNetPrioritizerSubsystem>();
    if (IsValid(Subsystem) && !Subsystem->IsInitialized())
    {
        Subsystem->ApplyNetPrioritizerPreset(EIPGMapType::Default, 0.0f);
    }

    if (!bGameStarted && (GetNumPlayers() >= RequiredPlayersToStart))
    {
        TriggerGameStart();
    }
}

void AIrisTestGameMode::StartPlay()
{
    UE_LOG(LogTemp, Log, TEXT("Iris Test Game Mode Starts Play!!!"));
    return;
}

void AIrisTestGameMode::BeginPlay()
{
    Super::BeginPlay(); 

    // Get Net Prioritizer Subsystem
    UIPGNetPrioritizerSubsystem* IPGNetPrioritizerSubsystem = GetWorld()->GetSubsystem<UIPGNetPrioritizerSubsystem>();
    if (!IsValid(IPGNetPrioritizerSubsystem))
    {
        return;
    }

    // Register Net Prioritizer Preset
    for (UIPGFoVNetPrioritizerDataAsset* FoVPrioritizerPreset : FoVPrioritizerPresets)
    {
        if (!IsValid(FoVPrioritizerPreset))
        {
            continue;
        }
        IPGNetPrioritizerSubsystem->RegisterNetPrioritizerPreset(FoVPrioritizerPreset);
    }

    // Apply default net prioritizer preset
    IPGNetPrioritizerSubsystem->ApplyNetPrioritizerPreset(EIPGMapType::Default, 0.0f);
}

void AIrisTestGameMode::TriggerGameStart()
{
    bGameStarted = true; 

    Super::StartPlay(); 

    GetWorldTimerManager().SetTimer(
        InsightsTimerHandle,
        this,
        &AIrisTestGameMode::StopProfilingAndShutdown,
        300.0f,
        false
    );
}

void AIrisTestGameMode::StopProfilingAndShutdown()
{
#if !WITH_EDITOR
    // Command to stop Unreal Insights file recording
    GEngine->Exec(GetWorld(), TEXT("trace.stop"));

    // Ensure the process shuts down completely 
    // so that the Docker container can detect the exit status code and execute the S3 upload
    FGenericPlatformMisc::RequestExit(false);
#endif
}
