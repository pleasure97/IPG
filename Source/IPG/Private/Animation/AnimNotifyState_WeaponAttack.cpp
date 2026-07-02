// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState_WeaponAttack.h"
#include "Combat/Weapon/IPGWeaponInterface.h"
#include "Combat/Weapon/WeaponHitDetectionComponent.h"

void UAnimNotifyState_WeaponAttack::SetCollisionEnabled(USkeletalMeshComponent* SkeletalMeshComponent, bool bEnabled)
{
	// Get the attacker which is owner from skeletal mesh component
	AActor* AttackOwner = SkeletalMeshComponent->GetOwner();
	// Check if the attacker is valid
	if (!IsValid(AttackOwner))
	{
		return;
	}

	// Search for child actors of the attacker
	TArray<AActor*> ChildActors;
	AttackOwner->GetAllChildActors(ChildActors, false);

	// Iterate child actors 
	for (AActor* ChildActor : ChildActors)
	{
		// Check if a child actor is valid
		if (!IsValid(ChildActor))
		{
			continue;
		}

		// Cast the child actor to weapon interface
		IIPGWeaponInterface* OwnerWithWeaponInterface = Cast<IIPGWeaponInterface>(ChildActor);
		if (!OwnerWithWeaponInterface)
		{
			continue;
		}

		// Get weapon hit detection component from weapon interface
		UWeaponHitDetectionComponent* WeaponHitDetection = OwnerWithWeaponInterface->GetWeaponHitDetection();
		if (!IsValid(WeaponHitDetection))
		{
			continue;
		}

		// Enable collision from the weapon hit detection component
		WeaponHitDetection->SetCollisionEnabled(bEnabled);
	}
}
