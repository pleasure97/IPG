// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/Grid/GridSlotWidget.h"
#include "GameplayTagContainer.h"
#include "EquipmentGridSlotWidget.generated.h"

class UEquipmentSlotItemWidget;
class UInventoryItem;
class UCommonLazyImage;
class UOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipmentGridSlotClicked, UEquipmentGridSlotWidget*, GridSlot, const FGameplayTag&, EquipmentTypeTag);

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UEquipmentGridSlotWidget : public UGridSlotWidget
{
	GENERATED_BODY()
	
public:
	void SetEquipmentSlotItem(UEquipmentSlotItemWidget* Item);

	UEquipmentSlotItemWidget* OnItemEquipped(UInventoryItem* Item, const FGameplayTag& InEquipmentTag, float TileSize);

	FEquipmentGridSlotClicked EquipmentGridSlotClicked;

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "GameItems.Equipment"))
	FGameplayTag EquipmentTypeTag;

	/* Base Widget */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonLazyImage> Image_GrayedOutIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Root;

	/* Slot Item */
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UEquipmentSlotItemWidget> EquipmentSlotItemClass;

	UPROPERTY()
	TObjectPtr<UEquipmentSlotItemWidget> EquipmentSlotItem;
};
