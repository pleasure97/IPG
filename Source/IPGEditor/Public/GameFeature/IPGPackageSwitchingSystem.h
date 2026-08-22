// Fill out your copyright notice in the Description page of Project Settings.
 
#pragma once
 
#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "IPGPackageSwitchingSystem.generated.h"

class UIPGPackageData;

DECLARE_MULTICAST_DELEGATE(FOnPackageSwitchCompleted);

UCLASS()
class IPGEDITOR_API UIPGPackageSwitchingSystem : public UEditorSubsystem
{
	GENERATED_BODY()
 
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
 
	bool UpdateSwitch(const UIPGPackageData* NewPackage);
 
	FOnPackageSwitchCompleted OnPackageSwitchCompleted;
 
private:
	void UpdateActivePackageChanged(const UIPGPackageData* NewPackage);
 
	static TSet<FString> ComputeDesiredPluginSet(const UIPGPackageData* Package);
 
	bool PrepareEditorForSwitch(const TSet<FString>& PluginsToUnmount);
 
	void UnmountPlugins(const TSet<FString>& PluginNames);
	void MountPlugins(const TSet<FString>& PluginNames);
 
	TSet<FString> CurrentPlugins;
 
	UPROPERTY(Transient)
	TSoftObjectPtr<UIPGPackageData> LastAppliedPackage;
 
	bool bSwitchInProgress = false;
 
	FDelegateHandle ActivePackageChangedHandle;
};