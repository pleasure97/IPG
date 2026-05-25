// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widget/SlotItem/SlotItemWidget.h"
#include "GameplayTagContainer.h"
#include "EquipmentSlotItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEquipmentSlotItemClicked, class UEquipmentSlotItemWidget*, SlottedItem);

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UEquipmentSlotItemWidget : public USlotItemWidget
{
	GENERATED_BODY()
	
public:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/* "EQUIPMENT TYPE TAG" Getter & Setter */
	void SetEquipmentTypeTag(const FGameplayTag& Tag);
	FGameplayTag GetEquipmentTypeTag() const;
	
	FEquipmentSlotItemClicked OnEquipmentSlotItemClicked;
private:
	UPROPERTY()
	FGameplayTag EquipmentTypeTag;
};
