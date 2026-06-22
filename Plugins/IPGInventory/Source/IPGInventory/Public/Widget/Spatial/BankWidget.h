// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/InventoryBaseWidget.h"
#include "BankWidget.generated.h"

class UCanvasPanel;
class UWidgetSwitcher;
class UInventoryCommonButton;
class UHoverItemWidget;
class UInventoryGridWidget;
class UItemDescriptionWidget;

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UBankWidget : public UInventoryBaseWidget
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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	/* Base widget */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGridWidget> Grid_Bank;

	/* Item Description */
	UItemDescriptionWidget* GetItemDescription();

	UFUNCTION()
	void ShowItemDescription(UInventoryItem* Item);

	void SetItemDescriptionSizeAndPosition(UItemDescriptionWidget* InItemDescription, UCanvasPanel* InCanvasPanel) const;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UItemDescriptionWidget> ItemDescriptionClass;

	UPROPERTY()
	TObjectPtr<UItemDescriptionWidget> ItemDescription;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DescriptionTimerDelay = 0.5f;

	FTimerHandle DescriptionTimer;
};
