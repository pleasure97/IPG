// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/IPGAttributeSet.h"
#include "Net/UnrealNetwork.h"

UIPGAttributeSet::UIPGAttributeSet()
{
}

void UIPGAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Iris Push Model
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, MaxHealth, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Health, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, MaxMana, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Mana, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, MaxStamina, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Stamina, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Strength, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Agility, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Intelligence, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, CriticalHitRate, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, CriticalHitMultiplier, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UIPGAttributeSet, Level, Params);
}

void UIPGAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	CLAMP_BY_MAX_ATTRIBUTE(Attribute, Health, MaxHealth);
	CLAMP_BY_MAX_ATTRIBUTE(Attribute, Mana, MaxMana);
	CLAMP_BY_MAX_ATTRIBUTE(Attribute, Stamina, MaxStamina);
}

void UIPGAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, MaxHealth, OldMaxHealth);
}

void UIPGAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Health, OldHealth);
}

void UIPGAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, MaxMana, OldMaxMana);
}

void UIPGAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Mana, OldMana);
}

void UIPGAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, MaxStamina, OldMaxStamina);
}

void UIPGAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Stamina, OldStamina);
}

void UIPGAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Strength, OldStrength);
}

void UIPGAttributeSet::OnRep_Agility(const FGameplayAttributeData& OldAgility) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Agility, OldAgility);
}

void UIPGAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Intelligence, OldIntelligence);
}

void UIPGAttributeSet::OnRep_CriticalHitRate(const FGameplayAttributeData& OldCriticalHitRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, CriticalHitRate, OldCriticalHitRate);
}

void UIPGAttributeSet::OnRep_CriticalHitMultiplier(const FGameplayAttributeData& OldCriticalHitMultiplier) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, CriticalHitMultiplier, OldCriticalHitMultiplier);
}

void UIPGAttributeSet::OnRep_Level(const FGameplayAttributeData& OldLevel) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UIPGAttributeSet, Level, OldLevel);
}
