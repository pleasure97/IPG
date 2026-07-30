#include "AssetTypeActions_CameraDirector.h"
#include "ActionCameraDirectorAsset.h"
#include "ActionCameraDirectorEditorToolkit.h"

#define LOCTEXT_NAMESPACE "ActionCameraEditor"

FAssetTypeActions_CameraDirector::FAssetTypeActions_CameraDirector(EAssetTypeCategories::Type InCategory)
    : Category(InCategory)
{
}

FText FAssetTypeActions_CameraDirector::GetName() const
{
    return LOCTEXT("AssetName", "Action Camera Director");
}

FColor FAssetTypeActions_CameraDirector::GetTypeColor() const
{
    return FColor(80.f, 180.f, 220.f);
}

UClass* FAssetTypeActions_CameraDirector::GetSupportedClass() const
{
    return UActionCameraDirectorAsset::StaticClass();;
}

uint32 FAssetTypeActions_CameraDirector::GetCategories()
{
    return Category;
}

void FAssetTypeActions_CameraDirector::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
    const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

    for (UObject* InObject : InObjects)
    {
        if (UActionCameraDirectorAsset* Asset = Cast<UActionCameraDirectorAsset>(InObject))
        {
            TSharedRef<FActionCameraDirectorEditorToolkit> NewToolkit = MakeShared<FActionCameraDirectorEditorToolkit>();
            NewToolkit->InitCameraDirector(Mode, EditWithinLevelEditor, Asset);
        }
    }
}

#undef LOCTEXT_NAMESPACE // ActionCameraEditor
