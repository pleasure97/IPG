// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemTraceComponent.generated.h"

class APlayerController;
class UInputMappingContext;
class UInputAction;
class UInventoryComponent;
class UInventoryHUDWidget;
struct FEnhancedInputActionEventBinding;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class IPGINVENTORY_API UItemTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UItemTraceComponent();

	UFUNCTION(BlueprintCallable)
	void ToggleInventory(); 

	UFUNCTION(BlueprintCallable)
	void PrimaryInteract();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void TraceItem();

	void CreateHUDWidget(); 

	/* Item Trace */
	UPROPERTY(EditAnywhere, Category = "Item Trace")
	double TraceLength = 500.0;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TEnumAsByte<ECollisionChannel> ItemTraceChannel = ECC_GameTraceChannel1;

	/* Input Mapping */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> PrimaryInteractAction;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;

	/* Widget */
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UInventoryHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UInventoryHUDWidget> HUDWidget;

	TWeakObjectPtr<APlayerController> OwnerPlayerController;
	TWeakObjectPtr<UInventoryComponent> InventoryComponent;
	TWeakObjectPtr<AActor> CurrentTracedActor;
	TWeakObjectPtr<AActor> LastTracedActor;
};
