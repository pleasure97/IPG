// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/EquipmentComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "IPGInventoryBPLibrary.h"
#include "Component/InventoryComponent.h"
#include "InventoryItem.h"
#include "Fragment/ItemFragment.h"
#include "Equipment/EquipmentActor.h"
#include "GameplayTagContainer.h"

void UEquipmentComponent::SetOwningSkeletalMesh(USkeletalMeshComponent* InOwningSkeletalMesh)
{
	OwningSkeletalMesh = InOwningSkeletalMesh;
}

void UEquipmentComponent::SetIsProxy(bool bIsProxy)
{
	bProxy = bIsProxy;
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	InitPlayerController();
}

void UEquipmentComponent::InitPlayerController()
{
	if (OwningPlayerController = Cast<APlayerController>(GetOwner()); OwningPlayerController.IsValid())
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
		{
			OnPossessedPawnChange(nullptr, OwnerCharacter);
		}
		else
		{
			OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UEquipmentComponent::OnPossessedPawnChange);
		}
	}
}

void UEquipmentComponent::OnPossessedPawnChange(APawn* OldPawn, APawn* NewPawn)
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(NewPawn); IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}

	InitInventoryComponent();
}

void UEquipmentComponent::InitInventoryComponent()
{
	InventoryComponent = UIPGInventoryBPLibrary::GetInventoryComponent(OwningPlayerController.Get()); 
	if (!InventoryComponent.IsValid())
	{
		return;
	}

	if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &UEquipmentComponent::OnItemEquipped))
	{
		InventoryComponent->OnItemEquipped.AddDynamic(this, &UEquipmentComponent::OnItemEquipped);
	}

	if (!InventoryComponent->OnItemUnequipped.IsAlreadyBound(this, &UEquipmentComponent::OnItemUnEquipped))
	{
		InventoryComponent->OnItemUnequipped.AddDynamic(this, &UEquipmentComponent::OnItemUnEquipped);
	}
}

void UEquipmentComponent::OnItemEquipped(UInventoryItem* EquippedItem)
{
	// Check if equipped item is valid
	if (IsValid(EquippedItem))
	{
		// Process at server side
		if (OwningPlayerController->HasAuthority())
		{
			// Get item manifest and its equipment fragment
			FItemManifest& EquippedItemManifest = EquippedItem->GetItemManifestMutable();
			FEquipmentFragment* EquipmentFragment = EquippedItemManifest.GetFragmentOfTypeMutable<FEquipmentFragment>();
			if (EquipmentFragment)
			{
				EquipmentFragment->OnEquip(OwningPlayerController.Get()); 

				// Spawn equipped actor according to equipment fragment, item spec, and owning skeletal mesh
				if (OwningSkeletalMesh.IsValid())
				{
					AEquipmentActor* SpawnedEquipmentActor = SpawnEquipmentActor(EquipmentFragment, EquippedItemManifest, OwningSkeletalMesh.Get());

					// Save spawned equipment actor to array 
					EquipmentActors.Add(SpawnedEquipmentActor);
				}
			}
		}
	}

}

void UEquipmentComponent::OnItemUnEquipped(UInventoryItem* UnEquippedItem)
{
	// Check if equipment item is valid
	if (IsValid(UnEquippedItem))
	{
		// Process at server side
		if (OwningPlayerController->HasAuthority())
		{
			// Get item manifest and its equipment fragment
			FItemManifest& UnEquipmentItemManifest = UnEquippedItem->GetItemManifestMutable(); 
			FEquipmentFragment* EquipmentFragment = UnEquipmentItemManifest.GetFragmentOfTypeMutable<FEquipmentFragment>(); 
			if (EquipmentFragment)
			{
				EquipmentFragment->OnUnequip(OwningPlayerController.Get());

				// Remove equipment actor with equipment type tag
				RemoveEquipmentActor(EquipmentFragment->GetEquipmentType());
			}
		}
	}
}

AEquipmentActor* UEquipmentComponent::SpawnEquipmentActor(FEquipmentFragment* EquipmentFragment, const FItemManifest& ItemSpec, USkeletalMeshComponent* AttachSkeletalMesh)
{
	if (!EquipmentFragment || !AttachSkeletalMesh)
	{
		return nullptr;
	}

	UWorld* World = AttachSkeletalMesh->GetWorld();
	TSubclassOf<AEquipmentActor> EquipmentActorClass = EquipmentFragment->GetEquipActorClass();

	// Spawn equipment actor using equipment fragment
	AEquipmentActor* SpawnedEquipmentActor = World->SpawnActorDeferred<AEquipmentActor>(EquipmentActorClass, FTransform::Identity);
	// Setup equipment actor information according to equipment fragment
	SpawnedEquipmentActor->SetOwner(GetOwner()); 
	SpawnedEquipmentActor->SetEquipmentType(EquipmentFragment->GetEquipmentType());
	// Set equipment actor of equipment fragment
	EquipmentFragment->SetEquipActor(SpawnedEquipmentActor);

	// GetOwner() is valid in AEquipActor::BeginPlay() because of SpawnActorDeferred()
	SpawnedEquipmentActor->FinishSpawning(FTransform::Identity);
	SpawnedEquipmentActor->AttachToComponent(
		AttachSkeletalMesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		EquipmentFragment->GetSocketAttachPoint());

	return SpawnedEquipmentActor;
}

AEquipmentActor* UEquipmentComponent::FindEquipmentActor(const FGameplayTag& InEquipmentTypeTag)
{
	auto FoundEquippedActor = EquipmentActors.FindByPredicate([&InEquipmentTypeTag](const AEquipmentActor* InEquippedActor)
		{
			return InEquippedActor->GetEquipmentType().MatchesTagExact(InEquipmentTypeTag);
		});

	return FoundEquippedActor ? *FoundEquippedActor : nullptr; return nullptr;
}

void UEquipmentComponent::RemoveEquipmentActor(const FGameplayTag& InEquipmentTypeTag)
{
	// Find equipment actor from equipment actors array 
	AEquipmentActor* FoundEquipmentActor = FindEquipmentActor(InEquipmentTypeTag);
	// Remove found equipment actor from array and destroy found equipment actor
	if (IsValid(FoundEquipmentActor))
	{
		EquipmentActors.Remove(FoundEquipmentActor);
		FoundEquipmentActor->Destroy();
	}
}