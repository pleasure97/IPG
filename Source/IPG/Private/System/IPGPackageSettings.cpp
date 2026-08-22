#include "System/IPGPackageSettings.h"
#include "AssetRegistry/AssetRegistryModule.h"

UIPGPackageSettings::UIPGPackageSettings()
{
	CategoryName = TEXT("IPG"); 
	SectionName = TEXT("Build Package");
}

const UIPGPackageData* UIPGPackageSettings::GetDeterminedActivePackage()
{
	if (bDeterminedPackageValid)
	{
		return DeterminedPackage.Get();
	}

	UIPGPackageData* Result = nullptr; 

	FString OverrideName;
	// Check whether `-BuildPackage=PackageName` is included in the command-line arguments
	if (FParse::Value(FCommandLine::Get(), TEXT("BuildPackage="), OverrideName) && !OverrideName.IsEmpty())
	{
		Result = FindPackageDataByName(OverrideName); 

		if (!IsValid(Result))
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot find the package data asset specified from '-BuildPackage=%s'"), *OverrideName);
		}
	}
	// Synchronously load ActivePackage assets selected by the developer in the project settings
	else
	{
		Result = GetDefault<UIPGPackageSettings>()->ActivePackage.LoadSynchronous();
	}
	
	DeterminedPackage = Result;
	bDeterminedPackageValid = true;

	return DeterminedPackage;
}

void UIPGPackageSettings::InvalidateDeterminedPackage()
{
	DeterminedPackage = nullptr;
	bDeterminedPackageValid = false;
}

#if WITH_EDITOR
void UIPGPackageSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Automatically called when a developer changes the ActivePackage value in the project settings 
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName(); 
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UIPGPackageSettings, ActivePackage))
	{
		InvalidateDeterminedPackage();
		// Communicate the new package information to other game feature policies that subscribe to this package
		OnActivePackageChanged.Broadcast(GetDeterminedActivePackage());
	}

}
#endif // WITH_EDITOR

UIPGPackageData* UIPGPackageSettings::FindPackageDataByName(const FString& InPackageName)
{
	const FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")); 

	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Wait for comandlet and cook initialization
	AssetRegistry.WaitForCompletion(); 

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByClass(UIPGPackageData::StaticClass()->GetClassPathName(), Assets, true /*bSearchSubclasses*/);

	// Return if asset data name is identical
	for (const FAssetData& Asset : Assets)
	{
		if (Asset.AssetName.ToString().Equals(InPackageName, ESearchCase::IgnoreCase))
		{
			return Cast<UIPGPackageData>(Asset.GetAsset());
		}
	}

	// Return if package name field is identical
	for (const FAssetData& Asset : Assets)
	{
		if (UIPGPackageData* IPGPackageData = Cast<UIPGPackageData>(Asset.GetAsset()))
		{
			if (IPGPackageData->PackageName.Equals(InPackageName, ESearchCase::IgnoreCase))
			{
				return IPGPackageData;
			}
		}
	}

	return nullptr;
}
