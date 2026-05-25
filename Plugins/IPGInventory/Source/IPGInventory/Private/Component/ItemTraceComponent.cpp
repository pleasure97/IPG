// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ItemTraceComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/HUD/InventoryHUDWidget.h"
#include "Component/ItemComponent.h"
#include "Component/InventoryComponent.h"

UItemTraceComponent::UItemTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UItemTraceComponent::ToggleInventory()
{
	if (!InventoryComponent.IsValid() || !IsValid(HUDWidget))
	{
		return;
	}

	InventoryComponent->ToggleInventoryMenu(InventoryComponent->IsMenuOpen());
	if (InventoryComponent->IsMenuOpen())
	{
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		HUDWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UItemTraceComponent::BeginPlay()
{
	Super::BeginPlay(); 

	// Save Owner Player Controller
	OwnerPlayerController = Cast<APlayerController>(GetOwner()); 
	if (!IsValid(OwnerPlayerController.Get()))
	{
		return;
	}

	// Add Input Mapping Context
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerPlayerController->GetLocalPlayer());
	if (IsValid(EnhancedInputLocalPlayerSubsystem))
	{
		EnhancedInputLocalPlayerSubsystem->AddMappingContext(DefaultInputMappingContext, 0);
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(OwnerPlayerController.Get()->InputComponent))
	{
		EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &UItemTraceComponent::PrimaryInteract);
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &UItemTraceComponent::ToggleInventory);
	}
	InventoryComponent = OwnerPlayerController.Get()->FindComponentByClass<UInventoryComponent>(); 

	CreateHUDWidget();
}

void UItemTraceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(OwnerPlayerController.Get()))
	{
		UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwnerPlayerController->GetLocalPlayer());
		if (IsValid(EnhancedInputLocalPlayerSubsystem))
		{
			EnhancedInputLocalPlayerSubsystem->RemoveMappingContext(DefaultInputMappingContext);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void UItemTraceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TraceItem();
}

void UItemTraceComponent::TraceItem()
{
	if (!IsValid(GEngine) || !IsValid(GEngine->GameViewport) || !IsValid(OwnerPlayerController.Get()))
	{
		return;
	}

	// Viewport Size 
	FVector2D ViewportSize;
	GEngine->GameViewport->GetViewportSize(ViewportSize); 
	// Viewport Center
	const FVector2D ViewportCenter = ViewportSize / 2.f;

	FVector TraceStart;
	FVector Forward; 

	if (!UGameplayStatics::DeprojectScreenToWorld(OwnerPlayerController.Get(), ViewportCenter, TraceStart, Forward))
	{
		return;
	}

	// Single Line Trace (Trace Start, Trace End)
	const FVector TraceEnd = TraceStart + Forward * TraceLength;
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ItemTraceChannel); 
	// Save Current and Last Traced Actor
	LastTracedActor = CurrentTracedActor;
	CurrentTracedActor = HitResult.GetActor(); 

	if (!CurrentTracedActor.IsValid())
	{
		if (IsValid(HUDWidget))
		{
			HUDWidget->HidePickupMessage();
		}
	}

	if (CurrentTracedActor == LastTracedActor)
	{
		return;
	}

	if (CurrentTracedActor.IsValid())
	{
		UItemComponent* ItemComponent = CurrentTracedActor->FindComponentByClass<UItemComponent>(); 
		if (!IsValid(ItemComponent))
		{
			return;
		}
		if (IsValid(HUDWidget))
		{
			HUDWidget->ShowPickupMessage(ItemComponent->GetPickupMessage()); 
		}
	}
}

void UItemTraceComponent::PrimaryInteract()
{
	if (!CurrentTracedActor.IsValid() || !InventoryComponent.IsValid())
	{
		return;
	}
	UItemComponent* ItemComponent = CurrentTracedActor->FindComponentByClass<UItemComponent>(); 
	if (!IsValid(ItemComponent))
	{
		return;
	}
	InventoryComponent->TryAddItem(ItemComponent);
}

void UItemTraceComponent::CreateHUDWidget()
{
	if (!OwnerPlayerController->IsLocalController())
	{
		return;
	}

	HUDWidget = CreateWidget<UInventoryHUDWidget>(OwnerPlayerController.Get(), HUDWidgetClass);
	if (IsValid(HUDWidget))
	{
		HUDWidget->AddToViewport();
	}
}

