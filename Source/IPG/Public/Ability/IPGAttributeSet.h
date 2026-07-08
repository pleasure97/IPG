// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "IPGAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)								\
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)					\
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)								\
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)								\
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)								\

#define CLAMP_BY_MAX_ATTRIBUTE(TargetAttribute, AttributeName, MaxAttributeName)	\
    if (TargetAttribute == Get##AttributeName##Attribute())							\
    {																				\
        NewValue = FMath::Clamp(NewValue, 0.f, Get##MaxAttributeName());			\
    }

/**
 * 
 */
UCLASS()
class IPG_API UIPGAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UIPGAttributeSet(); 

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/* Health */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Health);
	
	/* Mana */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, MaxMana);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Mana);

	/* Stamina */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Stamina);

	/* Stat */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Agility)
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Agility);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence)
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Intelligence);

	/* Critical Hit */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitRate)
	FGameplayAttributeData CriticalHitRate;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, CriticalHitRate);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_CriticalHitMultiplier)
	FGameplayAttributeData 	CriticalHitMultiplier;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, CriticalHitMultiplier);

	/* Level */
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level)
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, Level);

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData IncomingXP;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, IncomingXP);

	/* Damage */
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UIPGAttributeSet, IncomingDamage);

private:
	/* Health */
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	/* Mana */
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	/* Stamina */
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina) const;

	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina) const;

	/* Stat */
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldStrength) const;

	UFUNCTION()
	void OnRep_Agility(const FGameplayAttributeData& OldAgility) const;

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const;

	/* Critical Hit */
	UFUNCTION()
	void OnRep_CriticalHitRate(const FGameplayAttributeData& OldCriticalHitRate) const;

	UFUNCTION()
	void OnRep_CriticalHitMultiplier(const FGameplayAttributeData& OldCriticalHitMultiplier) const;

	/* Level */
	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldLevel) const;
};

