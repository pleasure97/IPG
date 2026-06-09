// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/InventoryBaseWidget.h"
#include "Type/InventoryGridType.h"
#include "GameplayTagContainer.h"
#include "SpatialInventoryWidget.generated.h"

class UItemComponent; 
class UInventoryItem;
class UCanvasPanel;
class UWidgetSwitcher;
class UInventoryCommonButton;
class UHoverItemWidget;
class UInventoryGridWidget;
class UEquipmentGridSlotWidget;
class UItemDescriptionWidget;
class UEquipmentSlotItemWidget;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API USpatialInventoryWidget : public UInventoryBaseWidget
{
	GENERATED_BODY()
	
public:
	/* Inventory Base Widget Override */
	virtual FSlotAvailabilityResult HasRoomForItem(UItemComponent* ItemComponent) const override;
	virtual void OnItemHovered(UInventoryItem* Item) override;
	virtual void OnItemUnhovered() override;
	virtual bool HasHoverItem() const override;
	virtual UHoverItemWidget* GetHoverItem() const override;
	virtual float GetTileSize() const override;
	/* Inventory Base Widget Override */

protected:
	/* UUserWidget Override */
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
private:
	/* Base widget */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	/* Inventory Grid */
	UFUNCTION()
	void ShowEquipment(); 

	UFUNCTION()
	void ShowConsumable(); 

	UFUNCTION()
	void ShowCraftable();

	UFUNCTION()
	void EquipmentGridSlotClicked(UEquipmentGridSlotWidget* EquipmentGridSlot, const FGameplayTag& EquipmentTypeTag);

	UFUNCTION()
	void EquipmentSlotItemClicked(UEquipmentSlotItemWidget* EquipmentSlotItem);

	void SetActiveGrid(UInventoryGridWidget* InventoryGrid, UInventoryCommonButton* Button);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> Grid_Equipment;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> Grid_Consumable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> Grid_Craftable;

	UPROPERTY()
	TArray<TObjectPtr<UEquipmentGridSlotWidget>> EquipmentGridSlots;

	TWeakObjectPtr<UInventoryGridWidget> ActiveGrid;

	/* Button */
	void DisableButton(UInventoryCommonButton* Button);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryCommonButton> Button_Equipment;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryCommonButton> Button_Consumable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryCommonButton> Button_Craftable;
	
	/* Item Description */
	UItemDescriptionWidget* GetItemDescription(); 
	UItemDescriptionWidget* GetEquipmentItemDescription();

	UFUNCTION()
	void ShowEquipmentItemDescription(UInventoryItem* Item);

	void SetItemDescriptionSizeAndPosition(UItemDescriptionWidget* InItemDescription, UCanvasPanel* InCanvasPanel) const;
	void SetEquipmentItemDescriptionSizeAndPosition(UItemDescriptionWidget* InItemDescription, UItemDescriptionWidget* InEquipmentItemDescription, UCanvasPanel* InCanvas) const;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UItemDescriptionWidget> ItemDescriptionClass;

	UPROPERTY()
	TObjectPtr<UItemDescriptionWidget> ItemDescription;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UItemDescriptionWidget> EquipmentItemDescriptionClass;

	UPROPERTY()
	TObjectPtr<UItemDescriptionWidget> EquipmentItemDescription;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float EquipmentDescriptionTimerDelay = 0.5f;

	FTimerHandle DescriptionTimer;
	FTimerHandle EquipmentDescriptionTimer;
	
	/* Slot Item */
	bool CanEquipHoverItem(UEquipmentGridSlotWidget* EquipmentGridSlot, const FGameplayTag& EquipmentTypeTag) const;
	UEquipmentGridSlotWidget* FindSlotWithEquipmentItem(UInventoryItem* EquipmentItem) const;
	void ClearItemSlot(UEquipmentGridSlotWidget* EquipmentGridSlot);
	void RemoveEquipmentSlotItem(UEquipmentSlotItemWidget* EquipmentSlotItem);
	void MakeEquipmentSlotItem(UEquipmentSlotItemWidget* EquipmentSlotItem, UEquipmentGridSlotWidget* EquipmentGridSlot, UInventoryItem* ItemToEquip); 
	void BroadcastSlotClickedDelegates(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip) const;
};
	
