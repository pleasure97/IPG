// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/ModuleManager.h"
#include "ToolMenus.h"
#include "GameFeature/IPGPackageSelectorToolbar.h"

class FIPGEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		// Must be registered after the tool menu system is ready
		// UToolMenus might not exist yet at the time of StartupModule()
		UToolMenus::RegisterStartupCallback(
			FSimpleMulticastDelegate::FDelegate::CreateStatic(&FIPGPackageSelector::Register));
	}

	virtual void ShutdownModule() override
	{
		UToolMenus::UnRegisterStartupCallback(this);
		FIPGPackageSelector::Unregister();
	}
};

IMPLEMENT_MODULE(FIPGEditorModule, IPGEditor);