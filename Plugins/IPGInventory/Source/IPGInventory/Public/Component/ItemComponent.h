// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Manifest/ItemManifest.h"
#include "ItemComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class IPGINVENTORY_API UItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UItemComponent();

	/* Item Manifest */
	void InitItemManifest(FItemManifest CopiedManifest);

	FItemManifest GetItemManifest() const { return ItemManifest; }
	FItemManifest& GetItemManifestMutable() { return ItemManifest; }

	/* Item Pickup */
	FString GetPickupMessage() const { return PickupMessage; }
	void PickedUp();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	bool IsSupportedForNetworking() const override { return true; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnPickedUp(); 

private:
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	FItemManifest ItemManifest;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FString PickupMessage;
};
