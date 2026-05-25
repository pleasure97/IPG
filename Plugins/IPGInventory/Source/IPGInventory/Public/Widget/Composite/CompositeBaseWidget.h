// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "CompositeBaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class IPGINVENTORY_API UCompositeBaseWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/* Fragment Tag */
	FGameplayTag GetFragmentTag() const;
	void SetFragmentTag(const FGameplayTag& Tag);

	virtual void Collapse();
	void Expand();

	using FuncType = TFunction<void(UCompositeBaseWidget*)>;
	virtual void ApplyFunction(FuncType Function) {}
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag FragmentTag;
};
