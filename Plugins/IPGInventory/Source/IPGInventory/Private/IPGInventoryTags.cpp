// Fill out your copyright notice in the Description page of Project Settings.


#include "IPGInventoryTags.h"

namespace IPGInventory
{
	/* User Widget*/
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_InventoryScreen, "UI.Widget.InventoryScreen")
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_BankScreen, "UI.Widget.BankScreen")
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_MerchantScreen, "UI.Widget.MerchantScreen")
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_TradeScreen, "UI.Widget.TradeScreen")
}
namespace FragmentTags
{
	/* Fragments */
	UE_DEFINE_GAMEPLAY_TAG(GridFragment, "Inventory.Fragment.GridFragment")
	UE_DEFINE_GAMEPLAY_TAG(IconFragment, "Inventory.Fragment.IconFragment")
	UE_DEFINE_GAMEPLAY_TAG(StackableFragment, "Inventory.Fragment.StackableFragment")
	UE_DEFINE_GAMEPLAY_TAG(ConsumableFragment, "Inventory.Fragment.ConsumableFragment")
	UE_DEFINE_GAMEPLAY_TAG(EquipmentFragment, "Inventory.Fragment.EquipmentFragment")

	/* Item type */
	UE_DEFINE_GAMEPLAY_TAG(Consumable, "Inventory.ItemType.Consumable")
	UE_DEFINE_GAMEPLAY_TAG(Equippable, "Inventory.ItemType.Equippable")
	UE_DEFINE_GAMEPLAY_TAG(Craftable, "Inventory.ItemType.Craftable")
}