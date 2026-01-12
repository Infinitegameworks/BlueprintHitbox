#pragma once

#include "CoreMinimal.h"
#include "HitboxDataAsset.h"

/**
 * Handles importing hitbox data from JSON files into UHitboxDataAsset.
 */
class BLUEPRINTHITBOXEDITOR_API FHitboxJsonImporter
{
public:
	/**
	 * Show the import dialog and create a new HitboxDataAsset
	 */
	static void ShowImportDialog();

	/**
	 * Import a JSON file and create a new asset
	 * @param JsonFilePath Path to the JSON file
	 * @param DestinationPath Content Browser path for the new asset
	 * @param AssetName Name for the new asset
	 * @param OutErrorMessage Error message if import fails
	 * @return The created asset, or nullptr if failed
	 */
	static UHitboxDataAsset* ImportFromFile(
		const FString& JsonFilePath,
		const FString& DestinationPath,
		const FString& AssetName,
		FString& OutErrorMessage
	);

	/**
	 * Reimport an existing asset from its source file
	 * @param Asset The asset to reimport
	 * @param OutErrorMessage Error message if reimport fails
	 * @return True if successful
	 */
	static bool ReimportAsset(UHitboxDataAsset* Asset, FString& OutErrorMessage);

	/**
	 * Parse JSON string into hitbox data
	 * @param JsonString The JSON content
	 * @param OutAnimations Parsed animation data
	 * @param OutErrorMessage Error message if parsing fails
	 * @return True if successful
	 */
	static bool ParseJsonToHitboxData(
		const FString& JsonString,
		TArray<FAnimationHitboxData>& OutAnimations,
		FString& OutErrorMessage
	);

private:
	static EHitboxType ParseHitboxType(const FString& TypeString);
	static FHitboxData ParseHitboxFromJson(const TSharedPtr<class FJsonObject>& JsonObject);
	static FSocketData ParseSocketFromJson(const TSharedPtr<class FJsonObject>& JsonObject);
	static FFrameHitboxData ParseFrameFromJson(const FString& FrameName, const TSharedPtr<class FJsonObject>& JsonObject);
};
