// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureData.h"
#include "IPGGameFeatureData.generated.h"

class UIPGGameFeatureTypeData;

/**
 * 
 */
UCLASS()
class IPG_API UIPGGameFeatureData : public UGameFeatureData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Game Feature")
	TSoftObjectPtr<UIPGGameFeatureTypeData> GameFeatureType;

#if WITH_EDITOR
	virtual void PostLoad() override;
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual bool CanEditChange(const FProperty* InProperty) const override;

#endif // WITH_EDITOR
	
private:
	void UpdateGameFeatureTypeData(); 

	FString GetPluginRoot() const;
};
