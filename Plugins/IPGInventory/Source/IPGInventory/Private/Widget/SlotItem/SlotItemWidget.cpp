// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotItem/SlotItemWidget.h"
#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "IPGInventoryBPLibrary.h"

/* "STACKABLE" Getter & Setter */
bool USlotItemWidget::IsStackable() const
{
	return bStackable;
}

void USlotItemWidget::SetStackable(bool bIsStackable)
{
	bStackable = bIsStackable;
}

/* "IMAGE ICON" Getter & Setter */
UCommonLazyImage* USlotItemWidget::GetImageIcon() const
{
	return Image_Icon;
}

/* "GRID INDEX" Getter & Setter */
void USlotItemWidget::SetGridIndex(int32 Index)
{
	GridIndex = Index;
}

int32 USlotItemWidget::GetGridIndex() const
{
	return GridIndex;
}

/* "GRID DIMENSIONS" Getter & Setter */
void USlotItemWidget::SetGridDimensions(const FIntPoint& Dimensions)
{
	GridDimensions = Dimensions;
}

FIntPoint USlotItemWidget::GetGridDimensions() const
{
	return GridDimensions;
}

/* "INVENTORY ITEM" Getter & Setter */
void USlotItemWidget::SetInventoryItem(UInventoryItem* Item)
{
	InventoryItem = Item;
}

UInventoryItem* USlotItemWidget::GetInventoryItem() const
{
	return InventoryItem.Get();
}

/* "IMAGE BRUSH" Setter */
void USlotItemWidget::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}

void USlotItemWidget::UpdateStackCoiunt(int32 StackCount)
{
	if (StackCount > 0)
	{
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Visible); 
		TextBlock_StackCount->SetText(FText::AsNumber(StackCount));
	}
	else
	{
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply USlotItemWidget::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnSlotItemClicked.Broadcast(GridIndex, MouseEvent);

	return FReply::Handled();
}

void USlotItemWidget::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	UIPGInventoryBPLibrary::ItemHovered(GetOwningPlayer(), InventoryItem.Get()); 
}

void USlotItemWidget::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	UIPGInventoryBPLibrary::ItemUnhovered(GetOwningPlayer());
}


