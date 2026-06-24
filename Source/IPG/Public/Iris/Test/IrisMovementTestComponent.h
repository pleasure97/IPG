// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IrisMovementTestComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPG_API UIrisMovementTestComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UIrisMovementTestComponent();

protected:
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void OnPawnPossessed(APawn* OldPawn, APawn* NewPawn);

	void ScheduleNextMovement();
	void ExecuteRandomMovement();
	void ExecuteRandomJump();

	TWeakObjectPtr<APlayerController> OwnerPC;
	TWeakObjectPtr<ACharacter> OwningCharacter;

	/* Timer */
	FTimerHandle MovementTimerHandle;
	FTimerHandle JumpTimerHandle;

	FVector2D CurrentMovementInput = FVector2D::ZeroVector;
	FVector StartLocation;

	/* Input */
	UPROPERTY(EditDefaultsOnly)
	float MinInputInterval = 0.5f; 

	UPROPERTY(EditDefaultsOnly)
	float MaxInputInterval = 1.5f;

	/* Jump */
	UPROPERTY(EditDefaultsOnly)
	float MinJumpInterval = 1.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxJumpInterval = 3.f;

	/* Radius */
	UPROPERTY(EditDefaultsOnly)
	float MaxMovementRadius = 1000.f;
};
