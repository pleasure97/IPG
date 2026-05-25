// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryArray.h"
#include "InventoryComponent.generated.h"

class UInventoryItem;
class UItemComponent;
class UInventoryBaseWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryItemChange, UInventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInInventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStackChange, const FSlotAvailabilityResult&, Result); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemEquipStatusChange, UInventoryItem*, Item); 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInventoryMenuToggle, bool, bOpen);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPGINVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent(); 

	void ToggleInventoryMenu(bool bOpen);

	bool IsMenuOpen() const { return bInventoryMenuOpen; }

	UInventoryBaseWidget* GetInventoryMenu() const;

	UFUNCTION(BlueprintCallable)
	void TryAddItem(UItemComponent* ItemComponent); 

	UFUNCTION(Server, Reliable)
	void Server_AddNewItem(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder); 

	UFUNCTION(Server, Reliable)
	void Server_AddStacksToItem(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder); 

	UFUNCTION(Server, Reliable)
	void Server_DropItem(UInventoryItem* Item, int32 StackCount); 

	UFUNCTION(Server, Reliable)
	void Server_ConsumeItem(UInventoryItem* Item, int32 StackCount);

	UFUNCTION(Server, Reliable)
	void Server_EquipSlotClicked(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip); 

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipSlotClicked(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip);

	/* Delegates */
	FInventoryItemChange OnItemAdded;
	FInventoryItemChange OnItemRemoved;
	FNoRoomInInventory NoRoomInInventory;
	FStackChange OnStackChanged;
	FItemEquipStatusChange OnItemEquipped;
	FItemEquipStatusChange OnItemUnequipped;
	FInventoryMenuToggle OnInventoryMenuToggled;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	bool IsSupportedForNetworking() const override { return true; }

private:
	void ConstructInventory();

	TWeakObjectPtr<APlayerController> OwningPlayerController;

	UPROPERTY(Replicated)
	FInventoryArray InventoryList; 

	/* Inventory Menu */
	UPROPERTY()
	TObjectPtr<UInventoryBaseWidget> InventoryMenu; 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInventoryBaseWidget> InventoryMenuClass;

	bool bInventoryMenuOpen;

	/* Drop Spawn Setup */
	void SpawnDroppedItem(UInventoryItem* Item, int32 StackCount);

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnAngleMin = -85.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnAngleMax = 85.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnDistanceMin = 10.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float DropSpawnDistanceMax = 50.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float RelativeSpawnElevation = 70.f;
};
