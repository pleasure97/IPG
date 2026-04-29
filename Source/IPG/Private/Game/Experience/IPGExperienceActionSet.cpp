// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Experience/IPGExperienceActionSet.h"
#include "GameFeatureAction.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UIPGExperienceActionSet::UIPGExperienceActionSet()
{
}

#if WITH_EDITOR
EDataValidationResult UIPGExperienceActionSet::IsDataValid(FDataValidationContext& Context) const
{
    EDataValidationResult DataValidationResult = CombineDataValidationResults(
        Super::IsDataValid(Context), EDataValidationResult::Valid);

    int32 EntryIndex = 0;

    for (const UGameFeatureAction* Action : Actions)
    {
        if (Action)
        {
            EDataValidationResult ChildDataValidationResult = Action->IsDataValid(Context);
            DataValidationResult = CombineDataValidationResults(DataValidationResult, ChildDataValidationResult);
        }
        else
        {
            DataValidationResult = EDataValidationResult::Invalid;
            Context.AddError(FText::FromString("Null entry in Actions"));
        }
        ++EntryIndex;
    }

    return DataValidationResult;
}
#endif

#if WITH_EDITORONLY_DATA
void UIPGExperienceActionSet::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif