// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Combat/Weapon/IPGWeaponInterface.h"
#include "IPGWeapon.generated.h"

class UWeaponHitDetectionComponent;
class UCapsuleComponent;

UCLASS()
class IPG_API AIPGWeapon : public AActor, public IIPGWeaponInterface
{
	GENERATED_BODY()
	
public:	
	AIPGWeapon();

	/* IPG Weapon Interface */
	virtual UMeshComponent* GetWeaponMesh() const override;
	virtual UShapeComponent* GetWeaponCollision() const override;
	virtual UWeaponHitDetectionComponent* GetWeaponHitDetection() const override;

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> WeaponMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UCapsuleComponent> WeaponCollision;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UWeaponHitDetectionComponent> WeaponHitDetection;
};
