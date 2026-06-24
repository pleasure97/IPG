// Fill out your copyright notice in the Description page of Project Settings.


#include "Iris/Test/IrisMovementTestComponent.h"
#include "GameFramework/Character.h"

UIrisMovementTestComponent::UIrisMovementTestComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UIrisMovementTestComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerPC = Cast<APlayerController>(GetOwner());
	OwnerPC->OnPossessedPawnChanged.AddDynamic(this, &UIrisMovementTestComponent::OnPawnPossessed);
	OwningCharacter = Cast<ACharacter>(OwnerPC->GetPawn());
	if (OwningCharacter.IsValid())
	{
		StartLocation = OwningCharacter->GetActorLocation();
	}

	if (OwnerPC.IsValid())
	{
		if (OwnerPC->HasAuthority() || OwnerPC->IsLocalController())
		{
			ScheduleNextMovement(); 

			float JumpTimerDelay = FMath::RandRange(MinJumpInterval, MaxJumpInterval);
			OwnerPC->GetWorldTimerManager().SetTimer(
				JumpTimerHandle,
				this,
				&UIrisMovementTestComponent::ExecuteRandomJump,
				JumpTimerDelay,
				true);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Owner Player Controller is NOT Valid in UIrisMovementTestComponent::BeginPlay()"));
	}
}

void UIrisMovementTestComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwnerPC.IsValid() && OwningCharacter.IsValid())
	{
		if (!CurrentMovementInput.IsZero())
		{
			FVector WorldMoveDirection(CurrentMovementInput.X, CurrentMovementInput.Y, 0.0f);
			OwningCharacter->AddMovementInput(WorldMoveDirection, 1.0f, true);
		}
	}
}

void UIrisMovementTestComponent::OnPawnPossessed(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(NewPawn))
	{
		OwningCharacter = Cast<ACharacter>(NewPawn);
		StartLocation = NewPawn->GetActorLocation();
	}
}

void UIrisMovementTestComponent::ScheduleNextMovement()
{
	if (!OwnerPC.IsValid())
	{
		return;
	}

	float Delay = FMath::FRandRange(MinInputInterval, MaxInputInterval);
	OwnerPC->GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&UIrisMovementTestComponent::ExecuteRandomMovement,
		Delay,
		false);
}

void UIrisMovementTestComponent::ExecuteRandomMovement()
{
	if (OwningCharacter.IsValid())
	{
		static const TArray<FVector2D> Directions = {
		   FVector2D(1, 0),   // W
		   FVector2D(-1, 0),  // S
		   FVector2D(0, 1),   // D
		   FVector2D(0, -1),  // A
		   FVector2D(1, 1).GetSafeNormal(), // WD
		   FVector2D(1, -1).GetSafeNormal(), // WA
		   FVector2D(-1, 1).GetSafeNormal(), // SD
		   FVector2D(-1, -1).GetSafeNormal(), // SA
		   FVector2D(0, 0),   // Stop
		};

		FVector2D SelectedInput = Directions[FMath::RandRange(0, Directions.Num() - 1)];

		FVector CurrentLocation = OwningCharacter->GetActorLocation();

		FVector2D CurrentLocation2D(CurrentLocation.X, CurrentLocation.Y);
		FVector2D StartLocation2D(StartLocation.X, StartLocation.Y);

		float DistanceFromCenter = FVector2D::Distance(CurrentLocation2D, StartLocation2D);

		if (DistanceFromCenter >= MaxMovementRadius)
		{
			// Calculate direction vector to start
			FVector2D DirectionToStart = (StartLocation2D - CurrentLocation2D).GetSafeNormal();

			SelectedInput = DirectionToStart;
		}
		CurrentMovementInput = SelectedInput;
	}

	ScheduleNextMovement();
}

void UIrisMovementTestComponent::ExecuteRandomJump()
{
	if (OwnerPC.IsValid() && OwningCharacter.IsValid())
	{
		OwningCharacter->Jump();
	}
}