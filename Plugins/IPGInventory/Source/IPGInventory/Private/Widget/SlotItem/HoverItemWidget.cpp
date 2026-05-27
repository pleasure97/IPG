// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotItem/HoverItemWidget.h"
#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "InventoryItem.h"

void UHoverItemWidget::SetImageBrush(const FSlateBrush& Brush) const
{
	Image_Icon->SetBrush(Brush);
}

FGameplayTag UHoverItemWidget::GetItemType() const
{
	if (InventoryItem.IsValid())
	{
		return InventoryItem->GetItemManifest().GetItemType();
	}
	return FGameplayTag();
}

/* Grid */
int32 UHoverItemWidget::GetPreviousGridIndex() const
{
	return PreviousGridIndex;
}

void UHoverItemWidget::SetPreviousGridIndex(int32 Index)
{
	PreviousGridIndex = Index;
}

FIntPoint UHoverItemWidget::GetGridDimensions() const
{
	return GridDimensions;
}

void UHoverItemWidget::SetGridDimensions(const FIntPoint& Dimensions)
{
	GridDimensions = Dimensions;
}

/* Stack */
void UHoverItemWidget::UpdateStackCount(const int32 Count)
{
	StackCount = Count;
	if (Count > 0)
	{
		TextBlock_StackCount->SetText(FText::AsNumber(Count)); 
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		TextBlock_StackCount->SetVisibility(ESlateVisibility::Collapsed);
	}
}

int32 UHoverItemWidget::GetStackCount() const
{
	return StackCount;
}

bool UHoverItemWidget::IsStackable() const
{
	return bStackable;
}

void UHoverItemWidget::SetStackable(bool bIsStackable)
{
	bStackable = bIsStackable;
}

/* Inventory Item */
UInventoryItem* UHoverItemWidget::GetInventoryItem() const
{
	return InventoryItem.Get();
}

void UHoverItemWidget::SetInventoryItem(UInventoryItem* Item)
{
	InventoryItem = Item;
}

