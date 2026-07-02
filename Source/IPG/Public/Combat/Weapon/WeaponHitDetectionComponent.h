// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/Weapon/WeaponHitDetectionDataAsset.h"
#include "WeaponHitDetectionComponent.generated.h"

class AIPGWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API UWeaponHitDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponHitDetectionComponent();

	// Would be called by anim notify state
	UFUNCTION(BlueprintCallable)
	void SetCollisionEnabled(bool bEnabled);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

	// Register weapon hit detection data asset in blueprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UWeaponHitDetectionDataAsset> HitDetectionData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FName WeaponParentBoneName = TEXT("hand_r");

private:
	void PerformSweepDetection();
	void PerformTriangleDetection();
	void PerformRawAnimDataDetection(float DeltaTime);

	TArray<FVector> GetPositionFromRawAnimData(FName SocketName, float DeltaTime);

	UPROPERTY()
	TMap<EHitDetectionMethod, UWeaponHitDetectionDataAsset*> PreloadedDataAssets;

	bool bCollisionActive = false; 
	bool bWeaponInitialized = false;

	// Cache socket location at previous frame
	TMap<FName/*Socket Name*/, FVector/*Socket Location*/> PrevSocketLocations;

	// Previous weapon transform for sweep
	FTransform PrevWeaponTransform;

	TWeakObjectPtr<UMeshComponent> OwnerWeaponMesh;
	TWeakObjectPtr<UShapeComponent> OwnerWeaponCollision;
	TWeakObjectPtr<USkeletalMeshComponent> OwnerCharacterSkeletalMesh;
	TWeakObjectPtr<UAnimInstance> OwnerCharacterAnimInstance;
};
