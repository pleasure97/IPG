// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Widget/InventoryBaseWidget.h"
#include "Component/ItemComponent.h"
#include "InventoryItem.h"
#include "Fragment/ItemFragment.h"
#if UE_WITH_IRIS
#include "Net/Iris/ReplicationSystem/ReplicationSystemUtil.h"
#include "Net/Iris/ReplicationSystem/EngineReplicationBridge.h"
#include "Iris/ReplicationSystem/ReplicationSystem.h"
#include "Iris/ReplicationSystem/NetRefHandle.h"
#include "Iris/ReplicationSystem/Filtering/NetObjectFilter.h"
#endif

UInventoryComponent::UInventoryComponent()
	: InventoryList(this) // Set Inventory Component to Owner of Inventory Fast Array Serializer 
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); 
	bInventoryMenuOpen = false;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Iris Push Model
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UInventoryComponent, InventoryList, Params);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay(); 

	ConstructInventory();

#if UE_WITH_IRIS
	// Apply filter to server only 
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		ApplyIrisOwnerFilter(); 
		ApplyIrisStaticPriority();
	}
#endif
}

void UInventoryComponent::ConstructInventory()
{
	OwningPlayerController = Cast<APlayerController>(GetOwner()); 
	checkf(OwningPlayerController.IsValid(), TEXT("Inventory component should have a PlayerController as owner."));
	if (!OwningPlayerController->IsLocalController())
	{
		return;
	}

	// TODO 
	InventoryMenu = CreateWidget<UInventoryBaseWidget>(OwningPlayerController.Get(), InventoryMenuClass); 
	InventoryMenu->AddToViewport(); 

	ToggleInventoryMenu(false /*bOpen*/);
}

#if UE_WITH_IRIS
void UInventoryComponent::ApplyIrisOwnerFilter()
{
	// Check if owner is valid
	AActor* Owner = GetOwner(); 
	if (!IsValid(Owner))
	{
		return;
	}

	// Check if replication system is valid
	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(Owner);
	if (!IsValid(ReplicationSystem))
	{
		return;
	}

	// Check if replication bridge is valid
	UEngineReplicationBridge* ReplicationBridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(Owner);
	if (!IsValid(ReplicationBridge))
	{
		return;
	}

	// Check if inventory component's net handle is valid
	UE::Net::FNetRefHandle ObjectNetHandle = ReplicationBridge->GetReplicatedRefHandle(Owner);
	if (!ObjectNetHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Object Net Handle is Not Valid in UInventoryComponent::ApplyIrisOwnerFilter()")); 
		return;
	}
	ReplicationSystem->SetFilter(ObjectNetHandle, UE::Net::ToOwnerFilterHandle);
}

void UInventoryComponent::ApplyIrisStaticPriority()
{
	// Check if owner is valid
	AActor* Owner = GetOwner(); 
	if (!IsValid(Owner))
	{
		return;
	}

	// Check if replication system is valid
	UReplicationSystem* ReplicationSystem = UE::Net::FReplicationSystemUtil::GetReplicationSystem(Owner);
	if (!IsValid(ReplicationSystem))
	{
		return;
	}

	// Check if replication bridge is valid
	UEngineReplicationBridge* ReplicationBridge = UE::Net::FReplicationSystemUtil::GetActorReplicationBridge(Owner);
	if (!IsValid(ReplicationBridge))
	{
		return;
	}

	// Check if inventory component's net handle is valid
	UE::Net::FNetRefHandle ObjectNetHandle = ReplicationBridge->GetReplicatedRefHandle(Owner);
	if (!ObjectNetHandle.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Object Net Handle is Not Valid in UInventoryComponent::ApplyIrisOwnerFilter()"));
		return;
	}

	ReplicationSystem->SetStaticPriority(ObjectNetHandle, 3.f);
}
#endif

void UInventoryComponent::ToggleInventoryMenu(bool bOpen)
{
	if (!IsValid(InventoryMenu) || !OwningPlayerController.IsValid())
	{
		return;
	}

	ESlateVisibility SlateVisibility = bOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	InventoryMenu->SetVisibility(SlateVisibility); 

	bInventoryMenuOpen = bOpen;

	if (bOpen)
	{
		FInputModeGameAndUI InputMode;
		OwningPlayerController->SetInputMode(InputMode);
	}
	else
	{
		FInputModeGameOnly InputMode;
		OwningPlayerController->SetInputMode(InputMode);
	}
	OwningPlayerController->SetShowMouseCursor(bOpen);
}

UInventoryBaseWidget* UInventoryComponent::GetInventoryMenu() const
{
	return InventoryMenu;
}

void UInventoryComponent::TryAddItem(UItemComponent* ItemComponent)
{
	FSlotAvailabilityResult Result = InventoryMenu->HasRoomForItem(ItemComponent);
	UInventoryItem* FoundItem = InventoryList.FindFirstItemByType(ItemComponent->GetItemManifest().GetItemType());
	
	Result.Item = FoundItem;
	if (Result.TotalRoomToFill == 0)
	{
		NoRoomInInventory.Broadcast();
		return;
	}

	if (Result.Item.IsValid() && Result.bStackable)
	{
		// Add stacks to an item that already exists in the inventory.
		// We only want to update the stack count, not create a new item of this type.
		OnStackChanged.Broadcast(Result); 
		Server_AddStacksToItem(ItemComponent, Result.TotalRoomToFill, Result.Remainder);
	}
	else if (Result.TotalRoomToFill > 0)
	{
		// This item type doesn't exist in the inventory. 
		// Create a new one and update all pertinent slots.
		Server_AddNewItem(ItemComponent, Result.bStackable ? Result.TotalRoomToFill : 0, Result.Remainder);
	}
}

void UInventoryComponent::Server_AddNewItem_Implementation(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder)
{
	UInventoryItem* NewItem = InventoryList.AddEntry(ItemComponent); 
	NewItem->SetTotalStackCount(StackCount);

#if UE_WITH_IRIS
	NewItem->ApplyIrisOwnerFilter(GetOwner());
#endif

	if (GetOwner()->GetNetMode() == ENetMode::NM_ListenServer || GetOwner()->GetNetMode() == ENetMode::NM_Standalone)
	{
		OnItemAdded.Broadcast(NewItem); 
	}

	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
		return;
	}

	FStackableFragment* StackableFragment = ItemComponent->GetItemManifestMutable().GetFragmentOfTypeMutable<FStackableFragment>();
	if (StackableFragment)
	{
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInventoryComponent::Server_AddStacksToItem_Implementation(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder)
{
	const FGameplayTag& ItemType = IsValid(ItemComponent) ? ItemComponent->GetItemManifest().GetItemType() : FGameplayTag::EmptyTag;
	UInventoryItem* Item = InventoryList.FindFirstItemByType(ItemType); 
	if (!IsValid(Item))
	{
		return;
	}

	Item->SetTotalStackCount(Item->GetTotalStackCount() + StackCount); 

	if (Remainder == 0)
	{
		ItemComponent->PickedUp();
		return;
	}
	
	FStackableFragment* StackableFragment = ItemComponent->GetItemManifestMutable().GetFragmentOfTypeMutable<FStackableFragment>();
	if (StackableFragment)
	{
		StackableFragment->SetStackCount(Remainder);
	}
}

void UInventoryComponent::Server_DropItem_Implementation(UInventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item); 
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount);
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(UInventoryComponent, InventoryList, this);

	SpawnDroppedItem(Item, StackCount);
}

void UInventoryComponent::Server_ConsumeItem_Implementation(UInventoryItem* Item, int32 StackCount)
{
	const int32 NewStackCount = Item->GetTotalStackCount() - StackCount;
	if (NewStackCount <= 0)
	{
		InventoryList.RemoveEntry(Item); 
	}
	else
	{
		Item->SetTotalStackCount(NewStackCount); 
	}

	if (FConsumableFragment* ConsumableFragment = Item->GetItemManifestMutable().GetFragmentOfTypeMutable<FConsumableFragment>())
	{
		ConsumableFragment->OnConsume(OwningPlayerController.Get());
	}
}

void UInventoryComponent::Server_EquipSlotClicked_Implementation(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip)
{
	Multicast_EquipSlotClicked(ItemToEquip, ItemToUnequip);
}

void UInventoryComponent::Multicast_EquipSlotClicked_Implementation(UInventoryItem* ItemToEquip, UInventoryItem* ItemToUnequip)
{
	OnItemEquipped.Broadcast(ItemToEquip); 
	OnItemUnequipped.Broadcast(ItemToUnequip);
}

void UInventoryComponent::SpawnDroppedItem(UInventoryItem* Item, int32 StackCount)
{
	const APawn* OwningPawn = OwningPlayerController->GetPawn(); 
	if (!IsValid(OwningPawn))
	{
		return;
	}

	// TODO - Change logic to suit game style
	FVector RotatedForward = OwningPawn->GetActorForwardVector(); 
	RotatedForward = RotatedForward.RotateAngleAxis(FMath::FRandRange(DropSpawnAngleMin, DropSpawnAngleMax), FVector::UpVector); 
	FVector SpawnLocation = OwningPawn->GetActorLocation() + RotatedForward * FMath::RandRange(DropSpawnDistanceMin, DropSpawnDistanceMax); 
	SpawnLocation.Z -= RelativeSpawnElevation;
	const FRotator SpawnRotation = FRotator::ZeroRotator;

	FItemManifest& ItemManifest = Item->GetItemManifestMutable();
	if (FStackableFragment* StackableFragment = ItemManifest.GetFragmentOfTypeMutable<FStackableFragment>())
	{
		StackableFragment->SetStackCount(StackCount);
	}
	ItemManifest.SpawnPickupActor(this, SpawnLocation, SpawnRotation);
}
