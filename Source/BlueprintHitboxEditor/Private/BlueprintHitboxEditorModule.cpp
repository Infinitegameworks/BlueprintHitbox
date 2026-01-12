#include "BlueprintHitboxEditorModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "HitboxDataAssetActions.h"
#include "ContentBrowserModule.h"
#include "ToolMenus.h"
#include "HitboxDataAsset.h"
#include "HitboxJsonImporter.h"

#define LOCTEXT_NAMESPACE "FBlueprintHitboxEditorModule"

void FBlueprintHitboxEditorModule::StartupModule()
{
	RegisterAssetTools();
	RegisterMenuExtensions();
}

void FBlueprintHitboxEditorModule::ShutdownModule()
{
	UnregisterAssetTools();
}

void FBlueprintHitboxEditorModule::RegisterAssetTools()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// Register asset type actions for HitboxDataAsset
	TSharedPtr<IAssetTypeActions> HitboxActions = MakeShareable(new FHitboxDataAssetActions());
	AssetTools.RegisterAssetTypeActions(HitboxActions.ToSharedRef());
	RegisteredAssetTypeActions.Add(HitboxActions);
}

void FBlueprintHitboxEditorModule::UnregisterAssetTools()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");
	if (AssetToolsModule)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();
		for (TSharedPtr<IAssetTypeActions>& Actions : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Actions.ToSharedRef());
		}
	}
	RegisteredAssetTypeActions.Empty();
}

void FBlueprintHitboxEditorModule::RegisterMenuExtensions()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateLambda([]()
	{
		// Add "Import Hitbox Data" to Content Browser context menu
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu");
		if (Menu)
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("ContentBrowserImportBasic");
			Section.AddMenuEntry(
				"ImportHitboxData",
				LOCTEXT("ImportHitboxData", "Import Hitbox Data"),
				LOCTEXT("ImportHitboxDataTooltip", "Import hitbox data from a JSON file exported by the Hitbox Editor"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.DataAsset"),
				FUIAction(FExecuteAction::CreateStatic(&FHitboxJsonImporter::ShowImportDialog))
			);
		}
	}));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlueprintHitboxEditorModule, BlueprintHitboxEditor)
