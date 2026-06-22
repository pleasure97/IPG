// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Spatial/BankWidget.h"
#include "Widget/Spatial/InventoryGridWidget.h"
#include "InventoryItem.h"
#include "Widget/Composite/ItemDescriptionWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "IPGInventoryBPLibrary.h"

FSlotAvailabilityResult UBankWidget::HasRoomForItem(UItemComponent* ItemComponent) const
{
    return Grid_Bank->HasRoomForItem(ItemComponent);
}

void UBankWidget::OnItemHovered(UInventoryItem* Item)
{
	const auto& Manifest = Item->GetItemManifest();
	UItemDescriptionWidget* ItemDescriptionWidget = GetItemDescription();
	if (!IsValid(ItemDescriptionWidget))
	{
		ItemDescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);

	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this, Item, &Manifest, ItemDescriptionWidget]()
		{
			GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible);
			Manifest.AssimilateInventoryFragments(ItemDescriptionWidget);

			// For second item description, show equip item of this type
			FTimerDelegate DescriptionTimerDelegate;
			DescriptionTimerDelegate.BindUObject(this, &UBankWidget::ShowItemDescription, Item);
			GetOwningPlayer()->GetWorldTimerManager().SetTimer(
				DescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay, false);
		});
}

void UBankWidget::OnItemUnhovered()
{
	UItemDescriptionWidget* ItemDescriptionWidget = GetItemDescription();
	FTimerManager& TimerManger = GetOwningPlayer()->GetWorldTimerManager();

	if (!IsValid(ItemDescriptionWidget))
	{
		return;
	}

	ItemDescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);
	TimerManger.ClearTimer(DescriptionTimer);
}

bool UBankWidget::HasHoverItem() const
{
	if (Grid_Bank->HasHoveredItem())
	{
		return true;
	}

	return false;
}

UHoverItemWidget* UBankWidget::GetHoverItem() const
{
    return Grid_Bank->GetHoverItem();
}

float UBankWidget::GetTileSize() const
{
	return Grid_Bank->GetTileSize();
}

void UBankWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Grid_Bank->SetOwningCanvas(CanvasPanel);
}

void UBankWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ItemDescription))
	{
		return;
	}

	SetItemDescriptionSizeAndPosition(ItemDescription, CanvasPanel);
}

UItemDescriptionWidget* UBankWidget::GetItemDescription()
{
	if (!IsValid(ItemDescription))
	{
		ItemDescription = CreateWidget<UItemDescriptionWidget>(GetOwningPlayer(), ItemDescriptionClass);
		CanvasPanel->AddChild(ItemDescription);
	}
	return ItemDescription;
}

void UBankWidget::ShowItemDescription(UInventoryItem* Item)
{
	if (!IsValid(Item))
	{
		return;
	}
	const auto& Manifest = Item->GetItemManifest();
	
	UItemDescriptionWidget* ItemDescriptionWidget = GetItemDescription();

	if (!IsValid(ItemDescriptionWidget))
	{
		return;
	}

	ItemDescriptionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UBankWidget::SetItemDescriptionSizeAndPosition(UItemDescriptionWidget* InItemDescription, UCanvasPanel* InCanvasPanel) const
{
	UCanvasPanelSlot* ItemDescriptionPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(InItemDescription);
	if (!IsValid(ItemDescriptionPanelSlot))
	{
		return;
	}

	const FVector2D ItemDescriptionSize = InItemDescription->GetBoxSize();
	ItemDescriptionPanelSlot->SetSize(ItemDescriptionSize);

	FVector2D ClampedPosition = UIPGInventoryBPLibrary::GetClampedWidgetPosition(
		UIPGInventoryBPLibrary::GetWidgetSize(CanvasPanel),
		ItemDescriptionSize,
		UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));

	ItemDescriptionPanelSlot->SetPosition(ClampedPosition);
}