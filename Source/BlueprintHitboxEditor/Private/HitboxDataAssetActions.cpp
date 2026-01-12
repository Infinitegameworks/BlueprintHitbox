#include "HitboxDataAssetActions.h"
#include "HitboxJsonImporter.h"
#include "HitboxDataAssetEditor.h"
#include "ToolMenuSection.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

#define LOCTEXT_NAMESPACE "HitboxDataAssetActions"

void FHitboxDataAssetActions::GetActions(const TArray<UObject*>& InObjects, FToolMenuSection& Section)
{
	TArray<TWeakObjectPtr<UHitboxDataAsset>> HitboxAssets;
	for (UObject* Object : InObjects)
	{
		if (UHitboxDataAsset* Asset = Cast<UHitboxDataAsset>(Object))
		{
			HitboxAssets.Add(Asset);
		}
	}

	// Reimport action
	Section.AddMenuEntry(
		"HitboxData_Reimport",
		LOCTEXT("Reimport", "Reimport from JSON"),
		LOCTEXT("ReimportTooltip", "Reimport hitbox data from the source JSON file"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Refresh"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FHitboxDataAssetActions::ExecuteReimport, HitboxAssets),
			FCanExecuteAction::CreateLambda([HitboxAssets]()
			{
				for (const TWeakObjectPtr<UHitboxDataAsset>& Asset : HitboxAssets)
				{
					if (Asset.IsValid() && !Asset->SourceFilePath.IsEmpty())
					{
						return true;
					}
				}
				return false;
			})
		)
	);

	// Show source file in explorer
	Section.AddMenuEntry(
		"HitboxData_ShowSource",
		LOCTEXT("ShowSource", "Show Source File"),
		LOCTEXT("ShowSourceTooltip", "Open the folder containing the source JSON file"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.FolderOpen"),
		FUIAction(
			FExecuteAction::CreateSP(this, &FHitboxDataAssetActions::ExecuteShowInExplorer, HitboxAssets),
			FCanExecuteAction::CreateLambda([HitboxAssets]()
			{
				for (const TWeakObjectPtr<UHitboxDataAsset>& Asset : HitboxAssets)
				{
					if (Asset.IsValid() && !Asset->SourceFilePath.IsEmpty() && FPaths::FileExists(Asset->SourceFilePath))
					{
						return true;
					}
				}
				return false;
			})
		)
	);
}

void FHitboxDataAssetActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	for (UObject* Object : InObjects)
	{
		if (UHitboxDataAsset* Asset = Cast<UHitboxDataAsset>(Object))
		{
			FHitboxDataAssetEditor::OpenEditor(Asset);
		}
	}
}

void FHitboxDataAssetActions::ExecuteReimport(TArray<TWeakObjectPtr<UHitboxDataAsset>> Objects)
{
	for (const TWeakObjectPtr<UHitboxDataAsset>& AssetPtr : Objects)
	{
		if (UHitboxDataAsset* Asset = AssetPtr.Get())
		{
			if (!Asset->SourceFilePath.IsEmpty())
			{
				FString ErrorMessage;
				if (FHitboxJsonImporter::ReimportAsset(Asset, ErrorMessage))
				{
					UE_LOG(LogTemp, Log, TEXT("Successfully reimported: %s"), *Asset->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to reimport %s: %s"), *Asset->GetName(), *ErrorMessage);
				}
			}
		}
	}
}

void FHitboxDataAssetActions::ExecuteShowInExplorer(TArray<TWeakObjectPtr<UHitboxDataAsset>> Objects)
{
	for (const TWeakObjectPtr<UHitboxDataAsset>& AssetPtr : Objects)
	{
		if (UHitboxDataAsset* Asset = AssetPtr.Get())
		{
			if (!Asset->SourceFilePath.IsEmpty() && FPaths::FileExists(Asset->SourceFilePath))
			{
				FPlatformProcess::ExploreFolder(*FPaths::GetPath(Asset->SourceFilePath));
				break;
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
