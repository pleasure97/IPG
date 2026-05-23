// Fill out your copyright notice in the Description page of Project Settings.


#include "Fragment/ItemFragment.h"

void FConsumableFragment::OnConsume(APlayerController* PC)
{
	for (TInstancedStruct<FConsumeModifier>& ConsumeModifier : ConsumeModifiers)
	{
		FConsumeModifier& ConsumeModifierRef = ConsumeModifier.GetMutable();
		ConsumeModifierRef.OnConsume(PC);
	}
}

void FConsumableFragment::Manifest()
{
	FItemFragment::Manifest();
	for (TInstancedStruct<FConsumeModifier>& ConsumeModifier : ConsumeModifiers)
	{
		FConsumeModifier& ConsumeModifierRef = ConsumeModifier.GetMutable();
		ConsumeModifierRef.Manifest();
	}
}