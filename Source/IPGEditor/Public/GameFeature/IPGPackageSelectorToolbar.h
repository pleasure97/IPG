// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class SWidget;
class UIPGPackageData;

/**
 * Build Recipe Package selection dropdown in the Level Editor toolbar
 */
class FIPGPackageSelector
{
public:
	static void Register();
	static void Unregister();

private:
	static TSharedRef<SWidget> GenerateMenu();

	static FText GetLabel();
	static FText GetTooltip();
	static bool CanSwitchPackage();

	static void SelectPackage(TSoftObjectPtr<UIPGPackageData> Package);
	static bool IsPackageSelected(TSoftObjectPtr<UIPGPackageData> Package);

	//  Find all package data assets in the Asset Registry.
	static TArray<UIPGPackageData*> FindAllPackages();
};