// Fill out your copyright notice in the Description page of Project Settings.


#include "Iris/IPGNetPrioritizerVolume.h"
#include "Iris/IPGNetPrioritizerSubsystem.h"

AIPGNetPrioritizerVolume::AIPGNetPrioritizerVolume()
{
	bReplicates = false;
}

void AIPGNetPrioritizerVolume::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// Has authority
	if (!HasAuthority())
	{
		return;
	}

	uint32 ConnectionId = 0; 
	if (!TryGetConnectId(OtherActor, ConnectionId))
	{
		return;
	}

	// Get Net Prioritizer Subsystem
	UIPGNetPrioritizerSubsystem* IPGNetPrioritizerSubsystem = GetWorld()->GetSubsystem<UIPGNetPrioritizerSubsystem>(); 
	if (!IsValid(IPGNetPrioritizerSubsystem))
	{
		return;
	}

	IPGNetPrioritizerSubsystem->ApplyNetPrioritizerPresetForConnection(ConnectionId, ZoneType);
}

void AIPGNetPrioritizerVolume::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	// Has authority
	if (!HasAuthority())
	{
		return;
	}

	uint32 ConnectionId = 0;
	if (!TryGetConnectId(OtherActor, ConnectionId))
	{
		return;
	}

	// Get Net Prioritizer Subsystem
	UIPGNetPrioritizerSubsystem* IPGNetPrioritizerSubsystem = GetWorld()->GetSubsystem<UIPGNetPrioritizerSubsystem>();
	if (!IsValid(IPGNetPrioritizerSubsystem))
	{
		return;
	}

	IPGNetPrioritizerSubsystem->ClearConnectionPreset(ConnectionId);
}

bool AIPGNetPrioritizerVolume::TryGetConnectId(AActor* OtherActor, OUT uint32& ConnectionId) const
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!IsValid(Pawn))
	{
		return false;
	}

	const APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PC) || !PC->GetNetConnection())
	{
		return false;
	}

	const UE::Net::FConnectionHandle Handle = PC->GetNetConnection()->GetConnectionHandle();
	if (!Handle.IsValid())
	{
		return false;
	}

	ConnectionId = Handle.GetParentConnectionId();

	return true;
}
