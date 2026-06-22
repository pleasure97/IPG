// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace IPGInventory
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_InventoryScreen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_BankScreen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_MerchantScreen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_TradeScreen);
}

namespace FragmentTags
{
	/* Fragments */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GridFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(IconFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(StackableFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConsumableFragment)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(EquipmentFragment)

	/* Item type */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Consumable)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Equippable)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Craftable)
}
