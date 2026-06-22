// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Type/InventoryGridType.h"
#include "InventoryBaseWidget.generated.h"

class UItemComponent;
class UInventoryItem;
class UHoverItemWidget;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UInventoryBaseWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	virtual FSlotAvailabilityResult HasRoomForItem(UItemComponent* ItemComponent) const { return FSlotAvailabilityResult(); }
	virtual void OnItemHovered(UInventoryItem* Item) {}
	virtual void OnItemUnhovered() {}
	virtual bool HasHoverItem() const { return false; }
	virtual UHoverItemWidget* GetHoverItem() const { return nullptr; }
	virtual float GetTileSize() const { return 0.f; }
};
