// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryArray.h"
#include "BankComponent.generated.h"

class UInventoryItem;
class UItemComponent;
class UInventoryBaseWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBankItemChange, UInventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNoRoomInBank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBankStackChange, const FSlotAvailabilityResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBankMenuToggle, bool, bOpen);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPGINVENTORY_API UBankComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBankComponent();

	void ToggleInventoryMenu(bool bOpen);

	bool IsMenuOpen() const; 

	UInventoryBaseWidget* GetInventoryMenu() const;

	UFUNCTION(BlueprintCallable)
	void DepositItem(UItemComponent* ItemComponent);

	UFUNCTION(BlueprintCallable)
	void WithdrawItem(UItemComponent* ItemComponent);

	UFUNCTION(Server, Reliable)
	void Server_DepositItem(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	UFUNCTION(Server, Reliable)
	void Server_WithdrawItem(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder);

	/* Delegates */
	FBankItemChange OnItemAdded;
	FBankItemChange OnItemRemoved;
	FNoRoomInBank NoRoomInBank;
	FBankStackChange OnBankStackChanged;
	FBankMenuToggle OnBankMenuToggled;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	bool IsSupportedForNetworking() const override { return true; }

private:
	void ConstructInventory();

#if UE_WITH_IRIS
	void ApplyIrisOwnerFilter();
	void ApplyIrisStaticPriority();
#endif

	TWeakObjectPtr<APlayerController> OwningPlayerController;

	UPROPERTY(Replicated)
	FInventoryArray BankList;

	/* Bank Menu */
	UPROPERTY()
	TObjectPtr<UInventoryBaseWidget> BankMenu;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<UInventoryBaseWidget> BankMenuClass;

	bool bBankMenuOpen;
};
