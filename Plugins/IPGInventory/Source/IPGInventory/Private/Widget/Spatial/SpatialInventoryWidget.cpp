// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Spatial/SpatialInventoryWidget.h"
#include "Widget/HUD/InventoryCommonButton.h"
#include "Widget/Spatial/InventoryGridWidget.h"
#include "Widget/SlotItem/HoverItemWidget.h"
#include "Widget/SlotItem/EquipmentSlotItemWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Widget/Grid/EquipmentGridSlotWidget.h"
#include "Widget/Composite/ItemDescriptionWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "IPGInventoryBPLibrary.h"
#include "Component/InventoryComponent.h"
#include "InventoryItem.h"
#include "Fragment/ItemFragment.h"

/*----------------------------------------------------- Inventory Base Widget ----------------------------------------------------- */
FSlotAvailabilityResult USpatialInventoryWidget::HasRoomForItem(UItemComponent* ItemComponent) const
{
	// Get Item Category from Item Component, and Process to Check if there are Enough Room in Each Item Grid
	switch (UIPGInventoryBPLibrary::GetItemContegoryFromItemComponent(ItemComponent))
	{
	case EItemCategory::Equippable:
	{
		return Grid_Equipment->HasRoomForItem(ItemComponent);
	}
	case EItemCategory::Consumable:
	{
		return Grid_Consumable->HasRoomForItem(ItemComponent);
	}
	case EItemCategory::Craftable:
	{
		return Grid_Craftable->HasRoomForItem(ItemComponent);
	}
	default:
	{
		UE_LOG(LogTemp, Error, TEXT("Inventory Item Component does Not Have Valid Item Category."));
		return FSlotAvailabilityResult();
	}
	}
}

void USpatialInventoryWidget::OnItemHovered(UInventoryItem* Item)
{
	const FItemManifest& Manifest = Item->GetItemManifest(); 
	UItemDescriptionWidget* ItemDescriptionWidget = GetItemDescription();
	if (!IsValid(ItemDescriptionWidget))
	{
		return;
	}

	ItemDescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);

	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer); 
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(EquipmentDescriptionTimer); 

	FTimerDelegate DescriptionTimerDelegate; 
	DescriptionTimerDelegate.BindLambda([this, Item, &Manifest, ItemDescriptionWidget]()
		{
			GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible); 
			Manifest.AssimilateInventoryFragments(ItemDescriptionWidget);

			// For second item description, show equip item of this type
			FTimerDelegate EquipmentDescriptionTimerDelegate;
			EquipmentDescriptionTimerDelegate.BindUObject(this, &USpatialInventoryWidget::ShowEquipmentItemDescription, Item); 
			GetOwningPlayer()->GetWorldTimerManager().SetTimer(
				EquipmentDescriptionTimer, EquipmentDescriptionTimerDelegate, EquipmentDescriptionTimerDelay, false); 
		});

	GetOwningPlayer()->GetWorldTimerManager().SetTimer(DescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay, false);
}

void USpatialInventoryWidget::OnItemUnhovered()
{
	UItemDescriptionWidget* ItemDescriptionWidget = GetItemDescription();
	UItemDescriptionWidget* EquipmentItemDescriptionWidget = GetEquipmentItemDescription(); 
	FTimerManager& TimerManger = GetOwningPlayer()->GetWorldTimerManager();

	if (!IsValid(ItemDescriptionWidget) || !IsValid(EquipmentItemDescription))
	{
		return;
	}

	ItemDescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);
	TimerManger.ClearTimer(DescriptionTimer);
	EquipmentItemDescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);
	TimerManger.ClearTimer(EquipmentDescriptionTimer);
}

bool USpatialInventoryWidget::HasHoverItem() const
{
	if (Grid_Equipment->HasHoveredItem())
	{
		return true;
	}

	if (Grid_Consumable->HasHoveredItem())
	{
		return true;
	}

	if (Grid_Craftable->HasHoveredItem())
	{
		return true;
	}

	return false;
}

UHoverItemWidget* USpatialInventoryWidget::GetHoverItem() const
{
	if (!ActiveGrid.IsValid())
	{
		return nullptr;
	}
	return ActiveGrid->GetHoverItem();
}

float USpatialInventoryWidget::GetTileSize() const
{
	return Grid_Equipment->GetTileSize();
}

/*----------------------------------------------------- UUserWidget ----------------------------------------------------- */
void USpatialInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Equipment->OnClicked().AddUObject(this, &USpatialInventoryWidget::ShowEquipment); 
	Button_Consumable->OnClicked().AddUObject(this, &USpatialInventoryWidget::ShowConsumable); 
	Button_Craftable->OnClicked().AddUObject(this, &USpatialInventoryWidget::ShowCraftable); 

	Grid_Equipment->SetOwningCanvas(CanvasPanel); 
	Grid_Consumable->SetOwningCanvas(CanvasPanel); 
	Grid_Craftable->SetOwningCanvas(CanvasPanel); 

	ShowConsumable(); 

	WidgetTree->ForEachWidget([this](UWidget* Widget)
		{
			UEquipmentGridSlotWidget* EquipmentGridSlot = Cast<UEquipmentGridSlotWidget>(Widget); 
			if (IsValid(EquipmentGridSlot))
			{
				EquipmentGridSlots.Add(EquipmentGridSlot); 
				EquipmentGridSlot->EquipmentGridSlotClicked.AddDynamic(this, &USpatialInventoryWidget::EquipmentGridSlotClicked);
			}
		});
}

FReply USpatialInventoryWidget::NativeOnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent)
{
	if (!ActiveGrid.IsValid())
	{
		return FReply::Unhandled();
	}
	ActiveGrid->DropItem();
	return FReply::Handled();
}

void USpatialInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(ItemDescription))
	{
		return;
	}

	SetItemDescriptionSizeAndPosition(ItemDescription, CanvasPanel); 
	SetEquipmentItemDescriptionSizeAndPosition(ItemDescription, EquipmentItemDescription, CanvasPanel); 
}

/*----------------------------------------------------- Inventory Grid ----------------------------------------------------- */
void USpatialInventoryWidget::ShowEquipment()
{
	SetActiveGrid(Grid_Equipment, Button_Equipment);
}

void USpatialInventoryWidget::ShowConsumable()
{
	SetActiveGrid(Grid_Consumable, Button_Consumable);
}

void USpatialInventoryWidget::ShowCraftable()
{
	SetActiveGrid(Grid_Craftable, Button_Craftable);
}

void USpatialInventoryWidget::EquipmentGridSlotClicked(UEquipmentGridSlotWidget* EquipmentGridSlot, const FGameplayTag& EquipmentTypeTag)
{
	// Check if hover item can be equipped
	if (!CanEquipHoverItem(EquipmentGridSlot, EquipmentTypeTag))
	{
		return;
	}

	UHoverItemWidget* HoverItem = GetHoverItem(); 
	if (!IsValid(HoverItem))
	{
		return;
	}

	// Create equipment slot item and add it to equipment grid slot 
	const float TileSize = UIPGInventoryBPLibrary::GetInventoryWidget(GetOwningPlayer())->GetTileSize(); 
	UEquipmentSlotItemWidget* EquipmentSlotItem = EquipmentGridSlot->OnItemEquipped(HoverItem->GetInventoryItem(), EquipmentTypeTag, TileSize);
	if (!IsValid(EquipmentSlotItem))
	{
		return;
	}
	EquipmentSlotItem->OnEquipmentSlotItemClicked.AddDynamic(this, &USpatialInventoryWidget::EquipmentSlotItemClicked); 

	// Inform the server that we've equipped an item 
	UInventoryComponent* InventoryComponent = UIPGInventoryBPLibrary::GetInventoryComponent(GetOwningPlayer()); 
	check(IsValid(InventoryComponent)); 

	InventoryComponent->Server_EquipSlotClicked(HoverItem->GetInventoryItem(), nullptr); 

	Grid_Equipment->ClearHoverItem();
}

void USpatialInventoryWidget::EquipmentSlotItemClicked(UEquipmentSlotItemWidget* EquipmentSlotItem)
{
	if (!IsValid(EquipmentSlotItem))
	{
		return;
	}

	// Remove item description 
	UIPGInventoryBPLibrary::ItemUnhovered(GetOwningPlayer());

	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable())
	{
		return;
	}

	// Get item to equip
	UInventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;

	// Get item to unequip 
	UInventoryItem* ItemToUnequip = EquipmentSlotItem->GetInventoryItem();

	// Get unequipment grid slot holding the item
	UEquipmentGridSlotWidget* EquipmentGridSlot = FindSlotWithEquipmentItem(ItemToUnequip);

	// Clear equipment grid slot of the item 
	ClearItemSlot(EquipmentGridSlot); 

	Grid_Equipment->AssignHoverItem(ItemToUnequip); 

	RemoveEquipmentSlotItem(EquipmentSlotItem); 

	MakeEquipmentSlotItem(EquipmentSlotItem, EquipmentGridSlot, ItemToEquip); 

	BroadcastSlotClickedDelegates(ItemToEquip, ItemToUnequip);
}

void USpatialInventoryWidget::SetActiveGrid(UInventoryGridWidget* InventoryGrid, UInventoryCommonButton* Button)
{
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->HideCursor(); 
		ActiveGrid->OnHide();
	}
	ActiveGrid = InventoryGrid;
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->ShowCursor();
	}
	
	DisableButton(Button); 

	WidgetSwitcher->SetActiveWidget(InventoryGrid);
}

/*----------------------------------------------------- Button ----------------------------------------------------- */
void USpatialInventoryWidget::DisableButton(UInventoryCommonButton* Button)
{
	if (!IsValid(Button))
	{
		return;
	}

	Button_Equipment->SetIsEnabled(false);
	Button_Consumable->SetIsEnabled(false);
	Button_Craftable->SetIsEnabled(false);
	
	Button->SetIsEnabled(true);
}

/*----------------------------------------------------- Item Description ----------------------------------------------------- */
UItemDescriptionWidget* USpatialInventoryWidget::GetItemDescription()
{
	if (!IsValid(ItemDescription))
	{
		ItemDescription = CreateWidget<UItemDescriptionWidget>(GetOwningPlayer(), ItemDescriptionClass); 
		CanvasPanel->AddChild(ItemDescription); 
	}
	return ItemDescription;
}

UItemDescriptionWidget* USpatialInventoryWidget::GetEquipmentItemDescription()
{
	if (!IsValid(EquipmentItemDescription))
	{
		EquipmentItemDescription = CreateWidget<UItemDescriptionWidget>(GetOwningPlayer(), EquipmentItemDescriptionClass);
		CanvasPanel->AddChild(EquipmentItemDescription); 
	}
	return EquipmentItemDescription;
}

void USpatialInventoryWidget::ShowEquipmentItemDescription(UInventoryItem* Item)
{
	if (!IsValid(Item))
	{
		return;
	}
	const auto& Manifest = Item->GetItemManifest(); 
	const FEquipmentFragment* EquipmentFragment = Manifest.GetFragmentOfType<FEquipmentFragment>(); 
	if (!EquipmentFragment)
	{
		return;
	}

	const FGameplayTag HoveredEquipmentType = EquipmentFragment->GetEquipmentType(); 

	auto EquipmentGridSlot = EquipmentGridSlots.FindByPredicate([Item](const UEquipmentGridSlotWidget* GridSlot)
		{
			return GridSlot->GetInventoryItem() == Item;
		}); 

	// Hover item is already equipped, so we're already showing its item description
	if (EquipmentGridSlot != nullptr)
	{
		return;
	}

	// If it's not equipped, find equipment item with same equipmet type 
	auto FoundEquipmentGridSlot = EquipmentGridSlots.FindByPredicate([HoveredEquipmentType](const UEquipmentGridSlotWidget* GridSlot)
		{
			UInventoryItem* InventoryItem = GridSlot->GetInventoryItem().Get();
			return IsValid(InventoryItem) ? 
				InventoryItem->GetItemManifest().GetFragmentOfType<FEquipmentFragment>()->GetEquipmentType() == HoveredEquipmentType : false;
		});
	UEquipmentGridSlotWidget* EquipmentGridSlotWidget = FoundEquipmentGridSlot ? *FoundEquipmentGridSlot : nullptr; 
	if (!IsValid(EquipmentGridSlotWidget))
	{
		return;
	}

	UInventoryItem* EquipmentItem = EquipmentGridSlotWidget->GetInventoryItem().Get(); 
	if (!IsValid(EquipmentItem))
	{
		return;
	}

	const auto& EquipmentItemManifest = EquipmentItem->GetItemManifest(); 
	UItemDescriptionWidget* ItemDescriptionWidget = GetItemDescription(); 
	UItemDescriptionWidget* EquipmentDescriptionWidget = GetEquipmentItemDescription();

	if (!IsValid(ItemDescriptionWidget) || !IsValid(EquipmentDescriptionWidget))
	{
		return;
	}

	EquipmentDescriptionWidget->Collapse(); 
	ItemDescriptionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	EquipmentItemManifest.AssimilateInventoryFragments(EquipmentDescriptionWidget); 
}

void USpatialInventoryWidget::SetItemDescriptionSizeAndPosition(UItemDescriptionWidget* InItemDescription, UCanvasPanel* InCanvasPanel) const
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

void USpatialInventoryWidget::SetEquipmentItemDescriptionSizeAndPosition(UItemDescriptionWidget* InItemDescription, UItemDescriptionWidget* InEquipmentItemDescription, UCanvasPanel* InCanvas) const
{
	UCanvasPanelSlot* ItemDescriptionPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(InItemDescription);
	UCanvasPanelSlot* EquippedItemDescriptionPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(InEquipmentItemDescription);
	if (!IsValid(ItemDescriptionPanelSlot) || !IsValid(EquippedItemDescriptionPanelSlot))
	{
		return;
	}

	const FVector2D ItemDescriptionSize = InItemDescription->GetBoxSize();
	const FVector2D EquipmentItemDescriptionSize = InEquipmentItemDescription->GetBoxSize();

	FVector2D ClampedPosition = UIPGInventoryBPLibrary::GetClampedWidgetPosition(
		UIPGInventoryBPLibrary::GetWidgetSize(CanvasPanel),
		ItemDescriptionSize,
		UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));
	ClampedPosition.X -= EquipmentItemDescriptionSize.X;

	EquippedItemDescriptionPanelSlot->SetSize(EquipmentItemDescriptionSize);
	EquippedItemDescriptionPanelSlot->SetPosition(ClampedPosition);
}

/*----------------------------------------------------- Slot Item ----------------------------------------------------- */
bool USpatialInventoryWidget::CanEquipHoverItem(UEquipmentGridSlotWidget* EquipmentGridSlot, const FGameplayTag& EquipmentTypeTag) const
{
	if (!IsValid(EquipmentGridSlot) || EquipmentGridSlot->GetInventoryItem().IsValid())
	{
		return false;
	}

	UHoverItemWidget* HoverItem = GetHoverItem(); 
	if (!IsValid(HoverItem))
	{
		return false;
	}

	UInventoryItem* HeldItem = HoverItem->GetInventoryItem(); 
	bool bEquipmentCategory = (HeldItem->GetItemManifest().GetItemCategory() == EItemCategory::Equippable); 
	bool bMatchEquipmentTag = HeldItem->GetItemManifest().GetItemType().MatchesTag(EquipmentTypeTag);

	return HasHoverItem() && IsValid(HeldItem) && !HoverItem->IsStackable() && bEquipmentCategory && bMatchEquipmentTag;
}

UEquipmentGridSlotWidget* USpatialInventoryWidget::FindSlotWithEquipmentItem(UInventoryItem* EquipmentItem) const
{
	if (!IsValid(EquipmentItem))
	{
		return nullptr;
	}
	auto* FoundEqiupmentGridSlot = EquipmentGridSlots.FindByPredicate([EquipmentItem](const UEquipmentGridSlotWidget* GridSlot)
		{
			return GridSlot->GetInventoryItem() == EquipmentItem;
		});
	return FoundEqiupmentGridSlot ? *FoundEqiupmentGridSlot : nullptr;
}

void USpatialInventoryWidget::ClearItemSlot(UEquipmentGridSlotWidget* EquipmentGridSlot)
{
	if (IsValid(EquipmentGridSlot))
	{
		EquipmentGridSlot->SetEquipmentSlotItem(nullptr); 
		EquipmentGridSlot->SetInventoryItem(nullptr);
	}
}

void USpatialInventoryWidget::RemoveEquipmentSlotItem(UEquipmentSlotItemWidget* EquipmentSlotItem)
{
	if (!IsValid(EquipmentSlotItem))
	{
		return;
	}
	
	if (EquipmentSlotItem->OnEquipmentSlotItemClicked.IsAlreadyBound(this, &USpatialInventoryWidget::EquipmentSlotItemClicked))
	{
		EquipmentSlotItem->OnEquipmentSlotItemClicked.RemoveDynamic(this, &USpatialInventoryWidget::EquipmentSlotItemClicked);
	}
	EquipmentSlotItem->RemoveFromParent(); 
}

void USpatialInventoryWidget::MakeEquipmentSlotItem(UEquipmentSlotItemWidget* EquipmentSlotItem, UEquipmentGridSlotWidget* EquipmentGridSlot, UInventoryItem* ItemToEquip)
{
	if (!IsValid(EquipmentGridSlot))
	{
		return;
	}

	UEquipmentSlotItemWidget* SlotItem = EquipmentGridSlot->OnItemEquipped(
		ItemToEquip,
		EquipmentSlotItem->GetEquipmentTypeTag(),
		UIPGInventoryBPLibrary::GetInventoryWidget(GetOwningPlayer())->GetTileSize()); 
	if (!IsValid(SlotItem))
	{
		return;
	}
	SlotItem->OnEquipmentSlotItemClicked.AddDynamic(this, &USpatialInventoryWidget::EquipmentSlotItemClicked); 

	EquipmentGridSlot->SetEquipmentSlotItem(SlotItem); 
}

void USpatialInventoryWidget::BroadcastSlotClickedDelegates(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip) const
{
	UInventoryComponent* InventoryComponent = UIPGInventoryBPLibrary::GetInventoryComponent(GetOwningPlayer()); 
	check(IsValid(InventoryComponent)); 

	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUnequip);
}