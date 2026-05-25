// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Type/InventoryGridType.h"
#include "InventoryBaseWidget.generated.h"

class UItemComponent;
class UInventoryItem;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventoryBaseWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	virtual FSlotAvailabilityResult HasRoomForItem(UItemComponent* ItemComponent) const { return FSlotAvailabilityResult(); }
	virtual void OnItemHovered(UInventoryItem* Item) {}
	virtual void OnItemUnhovered() {}
};
