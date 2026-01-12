#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBlueprintHitboxEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterAssetTools();
	void UnregisterAssetTools();
	void RegisterMenuExtensions();

	TArray<TSharedPtr<class IAssetTypeActions>> RegisteredAssetTypeActions;
};
