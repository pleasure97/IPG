// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/IPGInputConfig.h"

UIPGInputConfig::UIPGInputConfig()
{
}

const UInputAction* UIPGInputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FIPGInputAction& Action : NativeInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}
	return nullptr;
}

const UInputAction* UIPGInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag) const
{
	for (const FIPGInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && (Action.InputTag == InputTag))
		{
			return Action.InputAction;
		}
	}

	return nullptr;
}



