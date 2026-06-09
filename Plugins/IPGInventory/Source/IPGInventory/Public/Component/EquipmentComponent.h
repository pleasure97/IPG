// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentComponent.generated.h"

class UInventoryComponent;
class APlayerController;
class USkeletalMeshComponent;
class UInventoryItem;
class AEquipmentActor;
struct FEquipmentFragment;
struct FItemManifest;
struct FGameplayTag;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPGINVENTORY_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void SetOwningSkeletalMesh(USkeletalMeshComponent* InOwningSkeletalMesh); 
	void SetIsProxy(bool bIsProxy); 

protected:
	virtual void BeginPlay() override;

private:
	void InitPlayerController(); 

	UFUNCTION()
	void OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn);

	void InitInventoryComponent();

	UFUNCTION()
	void OnItemEquipped(UInventoryItem* EquippedItem);

	UFUNCTION()
	void OnItemUnEquipped(UInventoryItem* UnEquippedItem);

	AEquipmentActor* SpawnEquipmentActor(FEquipmentFragment* EquipmentFragment, const FItemManifest& ItemSpec, USkeletalMeshComponent* AttachSkeletalMesh);

	AEquipmentActor* FindEquipmentActor(const FGameplayTag& InEquipmentTypeTag);

	void RemoveEquipmentActor(const FGameplayTag& InEquipmentTypeTag);

	UPROPERTY()
	TArray<TObjectPtr<AEquipmentActor>> EquipmentActors;

	TWeakObjectPtr<UInventoryComponent> InventoryComponent;
	TWeakObjectPtr<APlayerController> OwningPlayerController;
	TWeakObjectPtr<USkeletalMeshComponent> OwningSkeletalMesh;

	bool bProxy = false;
};
