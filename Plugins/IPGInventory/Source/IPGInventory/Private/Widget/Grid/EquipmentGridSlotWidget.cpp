// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Grid/EquipmentGridSlotWidget.h"
#include "Widget/SlotItem/HoverItemWidget.h"
#include "Widget/SlotItem/EquipmentSlotItemWidget.h"
#include "IPGInventoryBPLibrary.h"
#include "CommonLazyImage.h"
#include "Fragment/ItemFragment.h"
#include "InventoryItem.h"
#include "IPGInventoryTags.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UEquipmentGridSlotWidget::SetEquipmentSlotItem(UEquipmentSlotItemWidget* Item)
{
    EquipmentSlotItem = Item;
}

void UEquipmentGridSlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (!IsAvailable())
    {
        return;
    }

    UHoverItemWidget* HoverItem = UIPGInventoryBPLibrary::GetHoverItem(GetOwningPlayer()); 
    if (!IsValid(HoverItem))
    {
        return;
    }

    if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag))
    {
        SetOccupiedTexture();
        Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
    }

}

void UEquipmentGridSlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    if (!IsAvailable())
    {
        return;
    }

    UHoverItemWidget* HoverItem = UIPGInventoryBPLibrary::GetHoverItem(GetOwningPlayer());
    if (!IsValid(HoverItem))
    {
        return;
    }

    if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag))
    {
        SetUnoccupiedTexture();
        Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
    }
}

FReply UEquipmentGridSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    EquipmentGridSlotClicked.Broadcast(this, EquipmentTypeTag);
    return FReply::Handled();
}

UEquipmentSlotItemWidget* UEquipmentGridSlotWidget::OnItemEquipped(UInventoryItem* Item, const FGameplayTag& InEquipmentTag, float TileSize)
{
    // Check equipment type tag
    if (!InEquipmentTag.MatchesTagExact(EquipmentTypeTag))
    {
        return nullptr;
    }

    // Get grid dimensions
    const FGridFragment* GridFragment = GetFragment<FGridFragment>(Item, FragmentTags::GridFragment); 
    if (!GridFragment)
    {
        return nullptr;
    }
    const FIntPoint GridDimensions = GridFragment->GetGridSize();

    // Calculate draw size for equipment slot item
    const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2; 
    const FVector2D DrawSize = GridDimensions * IconTileWidth;

    // Create equipment slot item widget 
    EquipmentSlotItem = CreateWidget<UEquipmentSlotItemWidget>(GetOwningPlayer(), EquipmentSlotItemClass);
    if (!IsValid(EquipmentSlotItem))
    {
        return nullptr;
    }

    // Setup equipment slot item settings
    EquipmentSlotItem->SetInventoryItem(Item);
    EquipmentSlotItem->SetEquipmentTypeTag(EquipmentTypeTag); 
    EquipmentSlotItem->UpdateStackCount(0);

    SetInventoryItem(Item); 

    // Get image fragment
    const FImageFragment* ImageFragment = GetFragment<FImageFragment>(Item, FragmentTags::IconFragment); 
    if (!ImageFragment)
    {
        return nullptr;
    }
    
    // Setup equipment slot item design
    FSlateBrush SlateBrush;
    SlateBrush.SetResourceObject(ImageFragment->GetIcon()); 
    SlateBrush.DrawAs = ESlateBrushDrawType::Image;
    SlateBrush.ImageSize = DrawSize;

    EquipmentSlotItem->SetImageBrush(SlateBrush);

    // Add slot item as a child to overlay 
    Overlay_Root->AddChildToOverlay(EquipmentSlotItem); 

    // Get overlay position and size 
    FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry(); 
    const FVector2f OverlayPosition = OverlayGeometry.Position;
    auto OverlaySize = OverlayGeometry.GetLocalSize(); 

    // Calculate overlay padding
    const float LeftPadding = OverlaySize.X / 2.f - DrawSize.X / 2.f;
    const float TopPadding = OverlaySize.Y / 2.f - DrawSize.Y / 2.f;

    UOverlaySlot* OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(EquipmentSlotItem); 
    if (!IsValid(OverlaySlot))
    {
        return nullptr;
    }
    OverlaySlot->SetPadding(FMargin(LeftPadding, TopPadding)); 

    return EquipmentSlotItem;
}
