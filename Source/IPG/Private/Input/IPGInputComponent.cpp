// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/IPGInputComponent.h"

UIPGInputComponent::UIPGInputComponent()
{

}

void UIPGInputComponent::RemoveBindings(TArray<uint32>& BindingHandles)
{
	for (uint32 BindingHandle : BindingHandles)
	{
		RemoveBindingByHandle(BindingHandle);
	}
	BindingHandles.Reset();
}