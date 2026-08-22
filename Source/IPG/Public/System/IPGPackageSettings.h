// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameFeature/IPGPackageData.h"
#include "IPGPackageSettings.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnActivePackageChanged, const UIPGPackageData* /* NewPackage*/);

/**
 * 
 */
UCLASS(config = EditorPerProjectUserSettings, meta = (DisplayName = "IPG Package"))
class IPG_API UIPGPackageSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UIPGPackageSettings();

	UPROPERTY(EditAnywhere, config, Category = "Patch")
	TSoftObjectPtr<UIPGPackageData> ActivePackage;

	static UIPGPackageSettings& Get() { return *GetMutableDefault<UIPGPackageSettings>(); }

	const UIPGPackageData* GetDeterminedActivePackage(); 

	void InvalidateDeterminedPackage(); 

	FOnActivePackageChanged OnActivePackageChanged; 

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:
	UIPGPackageData* FindPackageDataByName(const FString& InPackageName); 

	UPROPERTY(Transient)
	TObjectPtr<UIPGPackageData> DeterminedPackage;

	bool bDeterminedPackageValid = false;
};
