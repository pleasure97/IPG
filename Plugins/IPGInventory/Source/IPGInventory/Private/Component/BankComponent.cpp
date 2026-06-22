// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/BankComponent.h"
#include "Net/UnrealNetwork.h"

UBankComponent::UBankComponent()
{

}

void UBankComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Iris Push Model
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UBankComponent, BankList, Params);
}

void UBankComponent::BeginPlay()
{
	Super::BeginPlay();


}

void UBankComponent::DepositItem(UItemComponent* ItemComponent)
{

}

void UBankComponent::WithdrawItem(UItemComponent* ItemComponent)
{

}

void UBankComponent::Server_DepositItem_Implementation(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder)
{

}

void UBankComponent::Server_WithdrawItem_Implementation(UItemComponent* ItemComponent, int32 StackCount, int32 Remainder)
{

}