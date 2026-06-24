// Fill out your copyright notice in the Description page of Project Settings.


#include "Iris/Test/IrisTestGameMode.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

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
}
