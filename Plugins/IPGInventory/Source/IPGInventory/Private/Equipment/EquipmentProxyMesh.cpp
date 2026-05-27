// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/EquipmentProxyMesh.h"
#include "Gameframework/Character.h"
#include "Component/EquipmentComponent.h"
AEquipmentProxyMesh::AEquipmentProxyMesh()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);

	RootComponent = CreateDefaultSubobject<USceneComponent>("Root"); 

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh"); 
	Mesh->SetupAttachment(RootComponent); 

	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("Equipment");
	EquipmentComponent->SetOwningSkeletalMesh(Mesh); 
	EquipmentComponent->SetIsProxy(true);
}

USkeletalMeshComponent* AEquipmentProxyMesh::GetMesh() const
{
	return Mesh;
}

void AEquipmentProxyMesh::BeginPlay()
{
	Super::BeginPlay(); 

	DelayedInitializeOwner(); 
}

void AEquipmentProxyMesh::DelayedInitializeOwner()
{
	UWorld* World = GetWorld(); 
	if (!IsValid(World))
	{
		DelayedInitialization(); 
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController(); 
	if (!IsValid(PC))
	{
		DelayedInitialization(); 
		return;
	}

	ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
	if (!IsValid(Character))
	{
		DelayedInitialization();
		return;
	}

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		DelayedInitialization();
		return;
	}

	SourceMesh = CharacterMesh;

}

void AEquipmentProxyMesh::DelayedInitialization()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &AEquipmentProxyMesh::DelayedInitialization); 
	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}
