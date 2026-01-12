#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "HitboxDataAsset.h"

/**
 * Asset type actions for HitboxDataAsset.
 * Defines how the asset appears in the Content Browser and its context menu options.
 */
class FHitboxDataAssetActions : public FAssetTypeActions_Base
{
public:
	// FAssetTypeActions_Base interface
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "HitboxDataAsset", "Hitbox Data"); }
	virtual FColor GetTypeColor() const override { return FColor(255, 100, 100); }
	virtual UClass* GetSupportedClass() const override { return UHitboxDataAsset::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, struct FToolMenuSection& Section) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;

private:
	void ExecuteReimport(TArray<TWeakObjectPtr<UHitboxDataAsset>> Objects);
	void ExecuteShowInExplorer(TArray<TWeakObjectPtr<UHitboxDataAsset>> Objects);
};
