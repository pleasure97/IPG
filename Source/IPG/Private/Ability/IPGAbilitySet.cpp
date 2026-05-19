// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/IPGAbilitySet.h"
#include "AbilitySystemComponent.h"

void FIPGAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FIPGAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FIPGAbilitySet_GrantedHandles::AddAttributeSet(UAttributeSet* AttributeSet)
{
	GrantedAttributeSets.Add(AttributeSet);
}

void FIPGAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAbilitySystemComponent* ASC)
{
	check(ASC);

	// Must be authoritative to give or take ability sets.
	if (!ASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles)
	{
		if (AbilitySpecHandle.IsValid())
		{
			ASC->ClearAbility(AbilitySpecHandle);
		}
	}
	
	for (const FActiveGameplayEffectHandle& GameplayEffectHandle : GameplayEffectHandles)
	{
		if (GameplayEffectHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(GameplayEffectHandle);
		}
	}

	for (UAttributeSet* AttributeSet : GrantedAttributeSets)
	{
		ASC->RemoveSpawnedAttribute(AttributeSet);
	}

	AbilitySpecHandles.Reset(); 
	GameplayEffectHandles.Reset(); 
	GrantedAttributeSets.Reset();
}

UIPGAbilitySet::UIPGAbilitySet()
{
}

void UIPGAbilitySet::GiveToAbilitySystem(UAbilitySystemComponent* ASC, FIPGAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const
{
	check(ASC);

	// Must be authoritative to give or take ability sets.
	if (!ASC->IsOwnerActorAuthoritative())
	{
		return;
	}

	// Grant attribute sets 
	for (int32 SetIndex = 0; SetIndex < GrantedAttributes.Num(); ++SetIndex)
	{
		const FIPGAbilitySet_AttributeSet& SetToGrant = GrantedAttributes[SetIndex];
		if (!IsValid(SetToGrant.AttributeSet))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedAttributeSet is not valid in UIPGAbilitySet::GiveToAbilitySystem()"));
			continue;
		}

		UAttributeSet* NewAttributeSet = NewObject<UAttributeSet>(ASC->GetOwner(), SetToGrant.AttributeSet);
		ASC->AddAttributeSetSubobject(NewAttributeSet); 
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAttributeSet(NewAttributeSet);
		}
	}

	// Grant gameplay abilities
	for (int32 AbilityIndex = 0; AbilityIndex < GrantedGameplayAbilities.Num(); ++AbilityIndex)
	{
		const FIPGAbilitySet_GameplayAbility& AbilityToGrant = GrantedGameplayAbilities[AbilityIndex]; 
		if (!IsValid(AbilityToGrant.Ability))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedAbility is not valid in UIPGAbilitySet::GiveToAbilitySystem()"));
			continue;
		}

		UGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<UGameplayAbility>(); 

		FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel); 
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag); 

		const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec); 
		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddAbilitySpecHandle(AbilitySpecHandle);
		}
	}

	// Grant gameplay effects
	for (int32 EffectIndex = 0; EffectIndex < GrantedGameplayEffects.Num(); ++EffectIndex)
	{
		const FIPGAbilitySet_GameplayEffect& EffectToGrant = GrantedGameplayEffects[EffectIndex];
		if (!IsValid(EffectToGrant.GameplayEffect))
		{
			UE_LOG(LogTemp, Error, TEXT("GrantedGameplayEffect is not valid in UIPGAbilitySet::GiveToAbilitySystem()"));
			continue;
		}

		const UGameplayEffect* GameplayEffect = EffectToGrant.GameplayEffect->GetDefaultObject<UGameplayEffect>(); 
		const FActiveGameplayEffectHandle GameplayEffectHandle = ASC->ApplyGameplayEffectToSelf(GameplayEffect, EffectToGrant.EffectLevel, ASC->MakeEffectContext());

		if (OutGrantedHandles)
		{
			OutGrantedHandles->AddGameplayEffectHandle(GameplayEffectHandle);
		}
	}
}
