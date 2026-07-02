// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Weapon/WeaponHitDetectionComponent.h"
#include "Combat/Weapon/WeaponHitDetectionDataAsset.h"
#include "Combat/Weapon/IPGWeaponInterface.h"
#include "Combat/Weapon/IPGWeaponFunctionLibrary.h"
#include "Engine/AssetManager.h"
#include "Components/CapsuleComponent.h"

UWeaponHitDetectionComponent::UWeaponHitDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponHitDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get owner and cast it to weapon interface
	AActor* OwnerWeapon = GetOwner();
	IIPGWeaponInterface* OwnerWeaponInterface = Cast<IIPGWeaponInterface>(OwnerWeapon);
	if (!IsValid(OwnerWeapon) || !OwnerWeaponInterface)
	{
		return;
	}

	// Save owner weapon's mesh and collision
	OwnerWeaponMesh = OwnerWeaponInterface->GetWeaponMesh();
	OwnerWeaponCollision = OwnerWeaponInterface->GetWeaponCollision();

	// Get owner character
	AActor* OwnerCharacter = OwnerWeapon->GetOwner();
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	// TODO - Character can have multiple skeletal mesh components
	// Save owner character's skeletal mesh and anim instance
	OwnerCharacterSkeletalMesh = OwnerCharacter->FindComponentByClass<USkeletalMeshComponent>();
	if (!OwnerCharacterSkeletalMesh.IsValid())
	{
		return;
	}
	OwnerCharacterAnimInstance = OwnerCharacterSkeletalMesh->GetAnimInstance();
}

void UWeaponHitDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check if weapon's collision is active and the hit detection data asset is ready
	if (!bCollisionActive || !HitDetectionData)
	{
		return;
	}

	// Check if weapon mesh and collision are valid
	if (!OwnerWeaponMesh.IsValid() || !OwnerWeaponCollision.IsValid())
	{
		return;
	}

	// Perform hit detection
	switch (HitDetectionData->DetectionMethod)
	{
	case EHitDetectionMethod::SweepTrace:
		PerformSweepDetection();
		break;
	}

	// Update socket location
	for (const FWeaponSocketInfo& SocketInfo : HitDetectionData->Sockets)
	{
		PrevSocketLocations.Add(SocketInfo.SocketName, OwnerWeaponMesh->GetSocketLocation(SocketInfo.SocketName));
	}
}

void UWeaponHitDetectionComponent::SetCollisionEnabled(bool bEnabled)
{
	bCollisionActive = bEnabled;

	if (bEnabled)
	{
		// Reset previous position upon activation
		PrevSocketLocations.Reset();
		bWeaponInitialized = false;
	}
}

void UWeaponHitDetectionComponent::PerformSweepDetection()
{
	// Get weapon collision component 
	if (!OwnerWeaponCollision.IsValid() || !OwnerWeaponCollision->IsCollisionEnabled())
	{
		return;
	}

	// Get weapon location and rotation from capsule component
	FVector CurrentWeaponLocation = OwnerWeaponCollision->GetComponentLocation();
	FQuat CurrentWeaponRotation = OwnerWeaponCollision->GetComponentQuat();

	// Initialize the weapon's position and rotation
	if (!bWeaponInitialized)
	{
		PrevWeaponTransform.SetLocation(CurrentWeaponLocation);
		PrevWeaponTransform.SetRotation(CurrentWeaponRotation);
		bWeaponInitialized = true;
		return;
	}

	// Calculate a control point of the bezier curve
	FVector PrevWeaponLocation = PrevWeaponTransform.GetLocation();
	FQuat PrevWeaponRotation = PrevWeaponTransform.GetRotation();
	FVector TrajectoryVector = CurrentWeaponLocation - PrevWeaponLocation;
	FVector ForwardVector = TrajectoryVector.GetSafeNormal();
	FVector RightVector = FVector::CrossProduct(ForwardVector, FVector::UpVector);

	FVector ControlPoint = (PrevWeaponLocation + CurrentWeaponLocation) * 0.5f + RightVector * FVector::Dist(PrevWeaponLocation, CurrentWeaponLocation) * 0.5f;
	int NumSteps = FMath::CeilToInt(TrajectoryVector.Size() / HitDetectionData->InterpolationRate);

	UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(OwnerWeaponCollision.Get());

	for (int32 i = 0; i < NumSteps; ++i)
	{
		float InterpolationAlpha = (float) i / NumSteps;
		FVector InterpolatedPosition = UIPGWeaponFunctionLibrary::BezierInterpolate(PrevWeaponLocation, ControlPoint, CurrentWeaponLocation, InterpolationAlpha);
		FQuat InterpolatedRotation = FQuat::Slerp(PrevWeaponRotation, CurrentWeaponRotation, InterpolationAlpha);

		if (Capsule)
		{
			float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
			float CapsuleRadius = Capsule->GetScaledCapsuleRadius();

			DrawDebugCapsule(GetWorld(), InterpolatedPosition, CapsuleHalfHeight, CapsuleRadius, InterpolatedRotation, FColor::Green, false, 1.f);
		}
		// TODO - If you want to use box or sphere collision
	}

	PrevWeaponTransform.SetLocation(CurrentWeaponLocation);
	PrevWeaponTransform.SetRotation(CurrentWeaponRotation);
}