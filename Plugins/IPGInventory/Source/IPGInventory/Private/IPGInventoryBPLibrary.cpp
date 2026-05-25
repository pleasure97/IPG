// Copyright Epic Games, Inc. All Rights Reserved.

#include "IPGInventoryBPLibrary.h"
#include "Component/InventoryComponent.h"
#include "Widget/InventoryBaseWidget.h"

UInventoryComponent* UIPGInventoryBPLibrary::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	UInventoryComponent* InventoryComponent = PlayerController->FindComponentByClass<UInventoryComponent>();
	return InventoryComponent;
}

void UIPGInventoryBPLibrary::ItemHovered(APlayerController* PC, UInventoryItem* Item)
{
	// Get inventory component from player controller
	UInventoryComponent* InventoryComponent = GetInventoryComponent(PC); 
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	// Get inventory menu from inventory component 
	UInventoryBaseWidget* InventoryMenu = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryMenu))
	{
		return;
	}

	InventoryMenu->OnItemHovered(Item);
}

void UIPGInventoryBPLibrary::ItemUnhovered(APlayerController* PC)
{
	// Get inventory component from player controller
	UInventoryComponent* InventoryComponent = GetInventoryComponent(PC);
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	// Get inventory menu from inventory component 
	UInventoryBaseWidget* InventoryMenu = InventoryComponent->GetInventoryMenu();
	if (!IsValid(InventoryMenu))
	{
		return;
	}
	InventoryMenu->OnItemUnhovered();
}
