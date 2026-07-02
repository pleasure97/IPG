// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Weapon/IPGWeapon.h"
#include "Components/CapsuleComponent.h"

AIPGWeapon::AIPGWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	SetRootComponent(WeaponCollision);
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponCollision);

	WeaponHitDetection = CreateDefaultSubobject<UWeaponHitDetectionComponent>(TEXT("WeaponHitDetection"));
}

UMeshComponent* AIPGWeapon::GetWeaponMesh() const
{
	return WeaponMesh;
}

UShapeComponent* AIPGWeapon::GetWeaponCollision() const
{
	return WeaponCollision;
}

UWeaponHitDetectionComponent* AIPGWeapon::GetWeaponHitDetection() const
{
	return WeaponHitDetection;
}