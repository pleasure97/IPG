// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFeature/IPGPackageSelectorToolbar.h"
#include "GameFeature/IPGPackageData.h"
#include "GameFeature/IPGPackageSwitchingSystem.h"
#include "System/IPGPackageSettings.h"
#include "ToolMenus.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Editor.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "IPGPackageSelector"

void FIPGPackageSelector::Register()
{
	FToolMenuOwnerScoped OwnerScoped(TEXT("IPGPackageSelector"));

	UToolMenu* Toolbar = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.LevelEditorToolBar.User"));
	if (!Toolbar)
	{
		return;
	}

	FToolMenuSection& Section = Toolbar->FindOrAddSection(TEXT("IPGBuildRecipe"));

	FToolMenuEntry Entry = FToolMenuEntry::InitComboButton(
		TEXT("IPGPackageSelector"),
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateStatic(&FIPGPackageSelector::CanSwitchPackage)),
		FNewToolMenuChoice(FOnGetContent::CreateStatic(&FIPGPackageSelector::GenerateMenu)),
		TAttribute<FText>::CreateStatic(&FIPGPackageSelector::GetLabel),
		TAttribute<FText>::CreateStatic(&FIPGPackageSelector::GetTooltip),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Package")));

	Entry.StyleNameOverride = TEXT("CalloutToolbar");
	Section.AddEntry(Entry);
}

void FIPGPackageSelector::Unregister()
{
	if (UToolMenus* ToolMenus = UToolMenus::TryGet())
	{
		ToolMenus->UnregisterOwnerByName(TEXT("IPGPackageSelector"));
	}
}

FText FIPGPackageSelector::GetLabel()
{
	const UIPGPackageData* Active = UIPGPackageSettings::Get().GetDeterminedActivePackage();

	if (!IsValid(Active))
	{
		return LOCTEXT("PackageAll", "Package: All");
	}

	const FString DisplayName = Active->PackageName.IsEmpty() ? Active->GetName() : Active->PackageName;
	return FText::Format(LOCTEXT("PackageNamed", "Package: {0}"), FText::FromString(DisplayName));
}

FText FIPGPackageSelector::GetTooltip()
{
	if (GEditor && GEditor->PlayWorld != nullptr)
	{
		return LOCTEXT("TooltipDuringPIE", "플레이 중에는 패키지를 전환할 수 없습니다.");
	}

	const UIPGPackageData* Active = UIPGPackageSettings::Get().GetDeterminedActivePackage();
	if (IsValid(Active) && !Active->PackageDescription.IsEmpty())
	{
		return FText::FromString(Active->PackageDescription);
	}

	return LOCTEXT("TooltipDefault", "에디터에 마운트할 게임 피처 플러그인 세트를 선택합니다.");
}

bool FIPGPackageSelector::CanSwitchPackage()
{
	return GEditor == nullptr || GEditor->PlayWorld == nullptr;
}

TSharedRef<SWidget> FIPGPackageSelector::GenerateMenu()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, nullptr);

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("PackagesHeading", "Build Recipe Packages"));

	// "All" = No active package. Neither the policy nor the switcher performs filtering in this case
	MenuBuilder.AddMenuEntry(
		LOCTEXT("AllEntry", "All"),
		LOCTEXT("AllEntryTooltip", "설치된 모든 게임 피처 플러그인을 마운트합니다."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&FIPGPackageSelector::SelectPackage, TSoftObjectPtr<UIPGPackageData>()),
			FCanExecuteAction::CreateStatic(&FIPGPackageSelector::CanSwitchPackage),
			FIsActionChecked::CreateStatic(&FIPGPackageSelector::IsPackageSelected, TSoftObjectPtr<UIPGPackageData>())),
		NAME_None,
		EUserInterfaceActionType::RadioButton);

	for (UIPGPackageData* Package : FindAllPackages())
	{
		const FString DisplayName = Package->PackageName.IsEmpty() ? Package->GetName() : Package->PackageName;
		const TSoftObjectPtr<UIPGPackageData> PackagePtr(Package);

		MenuBuilder.AddMenuEntry(
			FText::FromString(DisplayName),
			FText::FromString(Package->PackageDescription),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateStatic(&FIPGPackageSelector::SelectPackage, PackagePtr),
				FCanExecuteAction::CreateStatic(&FIPGPackageSelector::CanSwitchPackage),
				FIsActionChecked::CreateStatic(&FIPGPackageSelector::IsPackageSelected, PackagePtr)),
			NAME_None,
			EUserInterfaceActionType::RadioButton);
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FIPGPackageSelector::SelectPackage(TSoftObjectPtr<UIPGPackageData> Package)
{
	UIPGPackageSettings& Settings = UIPGPackageSettings::Get();

	if (Settings.ActivePackage == Package)
	{
		return;
	}

	Settings.ActivePackage = Package;
	Settings.InvalidateDeterminedPackage();
	Settings.SaveConfig();

	Settings.OnActivePackageChanged.Broadcast(Settings.GetDeterminedActivePackage());
}

bool FIPGPackageSelector::IsPackageSelected(TSoftObjectPtr<UIPGPackageData> Package)
{
	return UIPGPackageSettings::Get().ActivePackage == Package;
}

TArray<UIPGPackageData*> FIPGPackageSelector::FindAllPackages()
{
	TArray<UIPGPackageData*> Result;

	const FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssetsByClass(
		UIPGPackageData::StaticClass()->GetClassPathName(), Assets, /*bSearchSubClasses=*/true);

	for (const FAssetData& Asset : Assets)
	{
		if (UIPGPackageData* Package = Cast<UIPGPackageData>(Asset.GetAsset()))
		{
			Result.Add(Package);
		}
	}

	Result.Sort([](const UIPGPackageData& A, const UIPGPackageData& B)
	{
		return A.PackageName < B.PackageName;
	});

	return Result;
}

#undef LOCTEXT_NAMESPACE