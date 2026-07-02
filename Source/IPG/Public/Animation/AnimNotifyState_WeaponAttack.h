// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_WeaponAttack.generated.h"

/**
 * Refer to 'ANS_WeaponAttack' in Blueprint
 * Enable collision at 'NotifyBegin' and disable it at 'NotifyEnd'
 */
UCLASS()
class IPG_API UAnimNotifyState_WeaponAttack : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetCollisionEnabled(USkeletalMeshComponent* SkeletalMeshComponent, bool bEnabled);
};
