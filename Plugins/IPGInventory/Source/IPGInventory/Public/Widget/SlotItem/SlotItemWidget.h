// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SlotItemWidget.generated.h"

class UCommonLazyImage;
class UCommonTextBlock;
class UInventoryItem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSlotItemClicked, int32, GridIndex, const FPointerEvent&, MouseEvent); 

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API USlotItemWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	/* "STACKABLE" Getter & Setter */
	bool IsStackable() const; 
	void SetStackable(bool bIsStackable); 

	/* "IMAGE ICON" Getter & Setter */
	UCommonLazyImage* GetImageIcon() const; 

	/* "GRID INDEX" Getter & Setter */
	void SetGridIndex(int32 Index);
	int32 GetGridIndex() const;

	/* "GRID DIMENSIONS" Getter & Setter */
	void SetGridDimensions(const FIntPoint& Dimensions);
	FIntPoint GetGridDimensions() const;

	/* "INVENTORY ITEM" Getter & Setter */
	void SetInventoryItem(UInventoryItem* Item);
	UInventoryItem* GetInventoryItem() const;

	/* "IMAGE BRUSH" Setter */
	void SetImageBrush(const FSlateBrush& Brush) const; 

	void UpdateStackCoiunt(int32 StackCount); 

	FSlotItemClicked OnSlotItemClicked;

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonLazyImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> TextBlock_StackCount;
	
	int32 GridIndex;
	FIntPoint GridDimensions; 
	TWeakObjectPtr<UInventoryItem> InventoryItem;
	bool bStackable = false;
};
