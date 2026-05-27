// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipmentProxyMesh.generated.h"

class UEquipmentComponent;

UCLASS()
class IPGINVENTORY_API AEquipmentProxyMesh : public AActor
{
	GENERATED_BODY()
	
public:
	AEquipmentProxyMesh(); 

	USkeletalMeshComponent* GetMesh() const; 

protected:
	virtual void BeginPlay() override; 

private:
	void DelayedInitializeOwner(); 
	void DelayedInitialization();

	// This is the mesh on player-controlled character
	TWeakObjectPtr<USkeletalMeshComponent> SourceMesh;

	// This is the proxy mesh we will see in inventory menu
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UEquipmentComponent> EquipmentComponent;

	FTimerHandle TimerForNextTick;
};
