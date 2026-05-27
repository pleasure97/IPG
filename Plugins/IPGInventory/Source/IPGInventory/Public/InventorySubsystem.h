// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "InventorySubsystem.generated.h"

class UInventoryCommonButton;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnButtonDescriptionTextUpdatedDelegate, UInventoryCommonButton*, BroadcastingButton, FText, DescriptionText);

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UInventorySubsystem* Get(const UObject* WorldContextObject);

	/* USubsystem Interface Begins */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	/* USubsystem Interface Ends */

	UPROPERTY(BlueprintAssignable)
	FOnButtonDescriptionTextUpdatedDelegate OnButtonDescriptionTextUpdated;
};
