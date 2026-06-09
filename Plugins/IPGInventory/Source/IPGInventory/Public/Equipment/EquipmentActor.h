// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/NetObjectGroupHandle.h"
#endif
#include "EquipmentActor.generated.h"

class UE::Net::FNetObjectGroupHandle;

UCLASS()
class IPGINVENTORY_API AEquipmentActor : public AActor
{
	GENERATED_BODY()
	
public:
	AEquipmentActor(); 

	FGameplayTag GetEquipmentType() const; 
	void SetEquipmentType(FGameplayTag Tag); 

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentTypeTag;

#if UE_WITH_IRIS
	void ApplyDistancePriority(); 
	void SuppressReplicationToOwner(APlayerController* OwnerPC);

	void ClearConnectionFilter();

	static constexpr int32 MaxConnections = 256;
	UE::Net::FNetObjectGroupHandle ConnectionFilterGroupHandle;
#endif
};
