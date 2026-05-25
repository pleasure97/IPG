// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/SlotItem/EquipmentSlotItemWidget.h"

FReply UEquipmentSlotItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    OnEquipmentSlotItemClicked.Broadcast(this);
    return FReply::Handled();
}

void UEquipmentSlotItemWidget::SetEquipmentTypeTag(const FGameplayTag& Tag)
{
    EquipmentTypeTag = Tag;
}

FGameplayTag UEquipmentSlotItemWidget::GetEquipmentTypeTag() const
{
    return EquipmentTypeTag;
}
