// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/Weapon/WeaponHitDetectionComponent.h"
#include "Combat/Weapon/WeaponHitDetectionDataAsset.h"
#include "Combat/Weapon/IPGWeaponInterface.h"
#include "Combat/Weapon/IPGWeaponFunctionLibrary.h"
#include "Engine/AssetManager.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/StaticMeshSocket.h"

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
	case EHitDetectionMethod::TriangleIntersection:
		PerformTriangleDetection();
		break;
	case EHitDetectionMethod::RawAnimationData:
		PerformRawAnimDataDetection(DeltaTime);
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

void UWeaponHitDetectionComponent::PerformTriangleDetection()
{
	if (!OwnerWeaponMesh.IsValid() || !HitDetectionData)
	{
		return;
	}

	//  Declare current socket information
	FVector CurrentTip;
	FVector CurrentRoot;
	bool bFoundTip = false;
	bool bFoundRoot = false;

	// Iterate socket information from the hit detection data asset
	for (const FWeaponSocketInfo& Info : HitDetectionData->Sockets)
	{
		// Save current weapon tip and root, as well as their positions
		FVector CurrentPosition = OwnerWeaponMesh->GetSocketLocation(Info.SocketName);
		if (Info.bWeaponTip)
		{
			CurrentTip = CurrentPosition;
			bFoundTip = true;
		}
		else
		{
			CurrentRoot = CurrentPosition;
			bFoundRoot = true;
		}
	}

	// When socket information is not correctly entered in hit detection data asset
	if (!bFoundTip || !bFoundRoot)
	{
		return;
	}

	// Initially, information for the tip and root should be recorded in an array
	if (!bWeaponInitialized)
	{
		PrevSocketLocations.Add(TEXT("Tip"), CurrentTip);
		PrevSocketLocations.Add(TEXT("Root"), CurrentRoot);
		bWeaponInitialized = true;
		return;
	}

	FVector PrevTip = PrevSocketLocations.FindRef(TEXT("Tip"));
	FVector PrevRoot = PrevSocketLocations.FindRef(TEXT("Root"));

	// Apply interpolation substeps
	int32 Steps = FMath::Max(1, HitDetectionData->InterpolationSubSteps);

	FVector LastSubTip = PrevTip;
	FVector LastSubRoot = PrevRoot;

	for (int32 i = 1; i <= Steps; i++)
	{
		float Alpha = (float)i / Steps;
		FVector InterpTip = FMath::Lerp(PrevTip, CurrentTip, Alpha);
		FVector InterpRoot = FMath::Lerp(PrevRoot, CurrentRoot, Alpha);

		// Define a single sub-step rectangle as two triangular vertices
		// First triangle vertex
		FVector FirstTriangle_A = LastSubRoot;
		FVector FirstTriangle_B = LastSubTip;
		FVector FirstTriangle_C = InterpTip;

		// Seocond triangle vertext
		FVector SecondTriangle_A = LastSubRoot;
		FVector SecondTriangle_B = InterpTip;
		FVector SecondTriangle_C = InterpRoot;

		// Draw triangle debug line 
		DrawDebugLine(GetWorld(), FirstTriangle_A, FirstTriangle_B, FColor::Magenta, false, 1.f, 0, 1.f);
		DrawDebugLine(GetWorld(), FirstTriangle_B, FirstTriangle_C, FColor::Magenta, false, 1.f, 0, 1.f);
		DrawDebugLine(GetWorld(), FirstTriangle_C, FirstTriangle_A, FColor::Magenta, false, 1.f, 0, 1.f);

		DrawDebugLine(GetWorld(), SecondTriangle_B, SecondTriangle_C, FColor::Magenta, false, 1.f, 0, 1.f);
		DrawDebugLine(GetWorld(), SecondTriangle_C, SecondTriangle_A, FColor::Magenta, false, 1.f, 0, 1.f);

		// Detect nearby target actors and perform triangle intersection tests.
		TArray<FHitResult> HitResults;
		FVector CenterPoint = (FirstTriangle_A + FirstTriangle_B + FirstTriangle_C + SecondTriangle_C) * 0.25f;
		float SearchRadius = FVector::Dist(FirstTriangle_A, FirstTriangle_B) * 1.5f; // Set radius

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Enemy (Pawn)
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetOwner()); // Excluding the instigator
		TArray<AActor*> OutActors;
		UKismetSystemLibrary::SphereOverlapActors(GetWorld(), CenterPoint, SearchRadius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

		for (AActor* TargetActor : OutActors)
		{
			// TODO - Check whether the enemy has collided with the two created triangular planes
		}

		// Update current position to Last for the next substep
		LastSubTip = InterpTip;
		LastSubRoot = InterpRoot;
	}

	// Cache update after frame completion
	PrevSocketLocations.Add(TEXT("Tip"), CurrentTip);
	PrevSocketLocations.Add(TEXT("Root"), CurrentRoot);
}

void UWeaponHitDetectionComponent::PerformRawAnimDataDetection(float DeltaTime)
{
	if (!OwnerWeaponMesh.IsValid() || !HitDetectionData)
	{
		return;
	}

	// Extract the entire arrays of substep positions for the tip and root from the original animation track
	TArray<FVector> SubStepTips = GetPositionFromRawAnimData(TEXT("WeaponTipSocket"), DeltaTime);
	TArray<FVector> SubStepRoots = GetPositionFromRawAnimData(TEXT("WeaponRootSocket"), DeltaTime);

	if (SubStepTips.Num() == 0 || SubStepRoots.Num() == 0)
	{
		return;
	}

	// Prevent initialization 
	if (!bWeaponInitialized)
	{
		PrevSocketLocations.Add(TEXT("Tip"), SubStepTips[0]);
		PrevSocketLocations.Add(TEXT("Root"), SubStepRoots[0]);
		bWeaponInitialized = true;
		return;
	}

	FVector LastSubTip = PrevSocketLocations.FindRef(TEXT("Tip"));
	FVector LastSubRoot = PrevSocketLocations.FindRef(TEXT("Root"));

	for (int32 i = 0; i < SubStepTips.Num(); ++i)
	{
		FVector InterpolationTip = SubStepTips[i];
		FVector InterpolationRoot = SubStepRoots[i];

		// 3. Define a single sub-step rectangle as two triangular vertices
		// First triangle vertex
		FVector FirstTriangle_A = LastSubRoot;
		FVector FirstTriangle_B = LastSubTip;
		FVector FirstTriangle_C = InterpolationTip;

		// Seocond triangle vertext
		FVector SecondTriangle_A = LastSubRoot;
		FVector SecondTriangle_B = InterpolationTip;
		FVector SecondTriangle_C = InterpolationRoot;

		// Draw triangle debug line 
		DrawDebugLine(GetWorld(), FirstTriangle_A, FirstTriangle_B, FColor::Red, false, 1.f, 0, 1.f);
		DrawDebugLine(GetWorld(), FirstTriangle_B, FirstTriangle_C, FColor::Red, false, 1.f, 0, 1.f);
		DrawDebugLine(GetWorld(), FirstTriangle_C, FirstTriangle_A, FColor::Red, false, 1.f, 0, 1.f);

		DrawDebugLine(GetWorld(), SecondTriangle_B, SecondTriangle_C, FColor::Red, false, 1.f, 0, 1.f);
		DrawDebugLine(GetWorld(), SecondTriangle_C, SecondTriangle_A, FColor::Red, false, 1.f, 0, 1.f);

		// 4. Detect nearby target actors and perform triangle intersection tests.
		TArray<FHitResult> HitResults;
		FVector CenterPoint = (FirstTriangle_A + FirstTriangle_B + FirstTriangle_C + SecondTriangle_C) * 0.25f;
		float SearchRadius = FVector::Dist(FirstTriangle_A, FirstTriangle_B) * 1.5f; // Set radius

		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn)); // Enemy (Pawn)
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(GetOwner()); // Excluding the instigator

		TArray<AActor*> OutActors;
		UKismetSystemLibrary::SphereOverlapActors(GetWorld(), CenterPoint, SearchRadius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

		for (AActor* TargetActor : OutActors)
		{
			// TODO - Check whether the enemy has collided with the two created triangular planes
		}

		// Update current position to Last for the next substep
		LastSubTip = InterpolationTip;
		LastSubRoot = InterpolationRoot;
	}

	// Cache update after frame completion
	PrevSocketLocations.Add(TEXT("Tip"), SubStepTips.Last());
	PrevSocketLocations.Add(TEXT("Root"), SubStepRoots.Last());
}

TArray<FVector> UWeaponHitDetectionComponent::GetPositionFromRawAnimData(FName SocketName, float DeltaTime)
{
	// Check if character anim instance and weapon mesh are valid
	if (!OwnerCharacterSkeletalMesh.IsValid() || !OwnerCharacterAnimInstance.IsValid() || !OwnerWeaponMesh.IsValid())
	{
		return TArray<FVector>();
	}

	// Get activated animation montage from character's anim instacne
	UAnimMontage* CurrentAnimMontage = OwnerCharacterAnimInstance->GetCurrentActiveMontage();
	if (!IsValid(CurrentAnimMontage))
	{
		return TArray<FVector>();
	}

	// Calculate current montage's timeline 
	float CurrentGlobalTime = OwnerCharacterAnimInstance->Montage_GetPosition(CurrentAnimMontage);

	UAnimSequence* TargetAnimSequence = nullptr;
	float SequencePosition = 0.f;

	// Convert the current montage timeline position to the actual sequence's local time 
	for (const FSlotAnimationTrack& SlotAnimationTrack : CurrentAnimMontage->SlotAnimTracks)
	{
		for (const FAnimSegment& AnimSegment : SlotAnimationTrack.AnimTrack.AnimSegments)
		{
			// Check if current montage time is within animation segment 
			if (CurrentGlobalTime >= AnimSegment.StartPos && CurrentGlobalTime <= AnimSegment.StartPos + AnimSegment.GetLength())
			{
				TargetAnimSequence = Cast<UAnimSequence>(AnimSegment.GetAnimReference());
				if (IsValid(TargetAnimSequence))
				{
					// Map montage global time to local time within the sequence (0 to animation length)
					// Calculate considering playback rate (AnimPlayRate) and start offset (AnimStartTime)
					float RelativeTime = CurrentGlobalTime - AnimSegment.StartPos;
					SequencePosition = AnimSegment.AnimStartTime + (RelativeTime * AnimSegment.AnimPlayRate);
					break;
				}
			}
		}
		if (TargetAnimSequence)
		{
			break;
		}
	}

	if (!TargetAnimSequence)
	{
		return TArray<FVector>();
	}

	float SequencePrevPosition = SequencePosition - DeltaTime;

	// Extract the socket's parent bone local offset
	FTransform SocketLocalOffset = FTransform::Identity;

	// Search for sockets in the character mesh
	if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(OwnerWeaponMesh.Get()))
	{
		if (UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh())
		{
			// Extract socket local offset from static mesh socket
			if (const UStaticMeshSocket* StaticSocket = StaticMesh->FindSocket(SocketName))
			{
				SocketLocalOffset = FTransform(StaticSocket->RelativeRotation, StaticSocket->RelativeLocation, StaticSocket->RelativeScale);
			}
		}
	}
	else if (USkeletalMeshComponent* SkeletaMeshComponent = Cast<USkeletalMeshComponent>(OwnerWeaponMesh.Get()))
	{
		// TODO
	}

	const USkeleton* Skeleton = TargetAnimSequence->GetSkeleton();
	if (!IsValid(Skeleton))
	{
		return TArray<FVector>();
	}

	// Get reference skeleton and find socket parent bone name
	const FReferenceSkeleton& ReferenceSkeleton = Skeleton->GetReferenceSkeleton();
	int32 BoneIndex = ReferenceSkeleton.FindBoneIndex(WeaponParentBoneName);
	if (BoneIndex == INDEX_NONE)
	{
		return TArray<FVector>();
	}

	// Subdivide time based on substeps and sample the actual pose
	FTransform WeaponAttachOffset = OwnerWeaponMesh->GetRelativeTransform();

	int32 InterpolationSubSteps = HitDetectionData->InterpolationSubSteps;

	TArray<FVector> ExtractedPositions;

	for (int32 i = 1; i <= InterpolationSubSteps; ++i)
	{
		float Alpha = (float)i / InterpolationSubSteps;
		float SubStepTime = FMath::Lerp(SequencePrevPosition, SequencePosition, Alpha);
		SubStepTime = FMath::Clamp(SubStepTime, 0.f, TargetAnimSequence->GetPlayLength());

		// Accumulate the entire bone chain and convert it to component space
		FTransform ComponentSpaceBoneTransform = FTransform::Identity;
		int32 CurrentBoneIndex = BoneIndex;
		const FAnimExtractContext Context(static_cast<double>(SubStepTime));

		while (CurrentBoneIndex != INDEX_NONE)
		{
			FTransform LocalBoneTransform;
			TargetAnimSequence->GetBoneTransform(LocalBoneTransform, FSkeletonPoseBoneIndex(CurrentBoneIndex), Context, false);

			// Accumulated from child to parent
			ComponentSpaceBoneTransform *= LocalBoneTransform;

			CurrentBoneIndex = ReferenceSkeleton.GetParentIndex(CurrentBoneIndex);
		}

		// Apply component space bone to socket local offset
		FTransform SocketInComponentSpace = SocketLocalOffset * ComponentSpaceBoneTransform;
		FTransform CharacterWorldTransform = OwnerCharacterSkeletalMesh->GetComponentTransform();
		FTransform SocketWorldTransform = SocketInComponentSpace * CharacterWorldTransform;

		FVector WorldSocketPosition = SocketWorldTransform.GetLocation();
		ExtractedPositions.Add(WorldSocketPosition);

		DrawDebugSphere(GetWorld(), WorldSocketPosition, 3.f, 6, FColor::Cyan, false, 1.f);
	}

	return ExtractedPositions;
}
