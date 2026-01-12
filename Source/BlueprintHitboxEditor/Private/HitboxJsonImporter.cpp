#include "HitboxJsonImporter.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "HitboxJsonImporter"

// Helper function to sanitize Content Browser paths
static FString SanitizePackagePath(const FString& InPath)
{
	FString Result = InPath;
	
	// UE5 Content Browser returns virtual paths like "/All/Game/..." 
	// We need actual package paths like "/Game/..."
	if (Result.StartsWith(TEXT("/All/")))
	{
		Result = Result.RightChop(4); // Remove "/All"
	}
	
	// Also handle "/All" without trailing content
	if (Result == TEXT("/All"))
	{
		Result = TEXT("/Game");
	}
	
	// Validate it's a proper package path
	if (!Result.StartsWith(TEXT("/")))
	{
		Result = TEXT("/Game");
	}
	
	// Final validation
	if (!FPackageName::IsValidLongPackageName(Result, false))
	{
		UE_LOG(LogTemp, Warning, TEXT("HitboxImporter: Invalid path '%s', defaulting to /Game"), *InPath);
		Result = TEXT("/Game");
	}
	
	UE_LOG(LogTemp, Log, TEXT("HitboxImporter: Sanitized path '%s' -> '%s'"), *InPath, *Result);
	return Result;
}

void FHitboxJsonImporter::ShowImportDialog()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) return;

	// Show file picker
	TArray<FString> SelectedFiles;
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Import Hitbox Data"),
		FPaths::ProjectContentDir(),
		TEXT(""),
		TEXT("JSON Files (*.json)|*.json"),
		EFileDialogFlags::None,
		SelectedFiles
	);

	if (bOpened && SelectedFiles.Num() > 0)
	{
		const FString& JsonFilePath = SelectedFiles[0];
		FString AssetName = FPaths::GetBaseFilename(JsonFilePath);
		
		// Get current Content Browser path
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		TArray<FString> SelectedPaths;
		ContentBrowserModule.Get().GetSelectedPathViewFolders(SelectedPaths);
		
		FString DestinationPath = TEXT("/Game");
		if (SelectedPaths.Num() > 0)
		{
			DestinationPath = SanitizePackagePath(SelectedPaths[0]);
		}

		FString ErrorMessage;
		UHitboxDataAsset* NewAsset = ImportFromFile(JsonFilePath, DestinationPath, AssetName, ErrorMessage);

		if (NewAsset)
		{
			// Select the new asset in Content Browser
			TArray<UObject*> AssetsToSync;
			AssetsToSync.Add(NewAsset);
			ContentBrowserModule.Get().SyncBrowserToAssets(AssetsToSync);
		}
		else
		{
			// Show error dialog
			FMessageDialog::Open(EAppMsgType::Ok, 
				FText::Format(LOCTEXT("ImportFailed", "Failed to import hitbox data:\n{0}"), 
				FText::FromString(ErrorMessage)));
		}
	}
}

UHitboxDataAsset* FHitboxJsonImporter::ImportFromFile(
	const FString& JsonFilePath,
	const FString& DestinationPath,
	const FString& AssetName,
	FString& OutErrorMessage)
{
	// Read file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *JsonFilePath);
		return nullptr;
	}

	// Parse JSON
	TArray<FAnimationHitboxData> Animations;
	if (!ParseJsonToHitboxData(JsonString, Animations, OutErrorMessage))
	{
		return nullptr;
	}

	// Create the asset
	FString PackagePath = DestinationPath / AssetName;
	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to create package: %s"), *PackagePath);
		return nullptr;
	}

	UHitboxDataAsset* NewAsset = NewObject<UHitboxDataAsset>(Package, *AssetName, RF_Public | RF_Standalone);
	if (!NewAsset)
	{
		OutErrorMessage = TEXT("Failed to create asset object");
		return nullptr;
	}

	// Populate the asset
	NewAsset->DisplayName = AssetName;
	NewAsset->Animations = Animations;
	NewAsset->SourceFilePath = JsonFilePath;
	NewAsset->LastImportTime = FDateTime::Now();

	// Mark dirty and save
	NewAsset->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewAsset);

	// Save the package
	FString PackageFilename = FPackageName::LongPackageNameToFilename(PackagePath, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, NewAsset, *PackageFilename, SaveArgs);

	return NewAsset;
}

bool FHitboxJsonImporter::ReimportAsset(UHitboxDataAsset* Asset, FString& OutErrorMessage)
{
	if (!Asset)
	{
		OutErrorMessage = TEXT("Invalid asset");
		return false;
	}

	if (Asset->SourceFilePath.IsEmpty())
	{
		OutErrorMessage = TEXT("No source file path stored");
		return false;
	}

	if (!FPaths::FileExists(Asset->SourceFilePath))
	{
		OutErrorMessage = FString::Printf(TEXT("Source file not found: %s"), *Asset->SourceFilePath);
		return false;
	}

	// Read file
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Asset->SourceFilePath))
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to read file: %s"), *Asset->SourceFilePath);
		return false;
	}

	// Parse JSON
	TArray<FAnimationHitboxData> Animations;
	if (!ParseJsonToHitboxData(JsonString, Animations, OutErrorMessage))
	{
		return false;
	}

	// Preserve Flipbook references
	TMap<FString, TSoftObjectPtr<UPaperFlipbook>> ExistingFlipbooks;
	for (const FAnimationHitboxData& Anim : Asset->Animations)
	{
		if (!Anim.Flipbook.IsNull())
		{
			ExistingFlipbooks.Add(Anim.AnimationName, Anim.Flipbook);
		}
	}

	// Update the asset
	Asset->Animations = Animations;
	Asset->LastImportTime = FDateTime::Now();

	// Restore Flipbook references
	for (FAnimationHitboxData& Anim : Asset->Animations)
	{
		if (TSoftObjectPtr<UPaperFlipbook>* Flipbook = ExistingFlipbooks.Find(Anim.AnimationName))
		{
			Anim.Flipbook = *Flipbook;
		}
	}

	Asset->MarkPackageDirty();

	return true;
}

bool FHitboxJsonImporter::ParseJsonToHitboxData(
	const FString& JsonString,
	TArray<FAnimationHitboxData>& OutAnimations,
	FString& OutErrorMessage)
{
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		OutErrorMessage = FString::Printf(TEXT("Failed to parse JSON: %s"), *Reader->GetErrorMessage());
		return false;
	}

	OutAnimations.Empty();

	// Iterate through root-level fields
	for (const auto& RootPair : RootObject->Values)
	{
		const FString& Key = RootPair.Key;
		const TSharedPtr<FJsonValue>& Value = RootPair.Value;

		if (Value->Type != EJson::Object) continue;

		TSharedPtr<FJsonObject> GroupOrFrameObject = Value->AsObject();

		// Detect format: grouped (animation -> frames) or flat (frame directly)
		bool bIsGroupedFormat = false;
		for (const auto& ChildPair : GroupOrFrameObject->Values)
		{
			if (ChildPair.Value->Type == EJson::Object)
			{
				TSharedPtr<FJsonObject> ChildObj = ChildPair.Value->AsObject();
				if (ChildObj->HasField(TEXT("hitboxes")) || ChildObj->HasField(TEXT("sockets")))
				{
					bIsGroupedFormat = true;
					break;
				}
			}
		}

		if (bIsGroupedFormat)
		{
			// Grouped format: { "AnimationName": { "frame1": {...}, "frame2": {...} } }
			FAnimationHitboxData AnimData;
			AnimData.AnimationName = Key;

			for (const auto& FramePair : GroupOrFrameObject->Values)
			{
				if (FramePair.Value->Type == EJson::Object)
				{
					FFrameHitboxData FrameData = ParseFrameFromJson(FramePair.Key, FramePair.Value->AsObject());
					AnimData.Frames.Add(FrameData);
				}
			}

			if (AnimData.Frames.Num() > 0)
			{
				OutAnimations.Add(AnimData);
			}
		}
		else
		{
			// Flat format: { "frame1": { "hitboxes": [...] } }
			FAnimationHitboxData* DefaultAnim = nullptr;

			for (FAnimationHitboxData& Anim : OutAnimations)
			{
				if (Anim.AnimationName == TEXT("Default"))
				{
					DefaultAnim = &Anim;
					break;
				}
			}

			if (!DefaultAnim)
			{
				FAnimationHitboxData NewAnim;
				NewAnim.AnimationName = TEXT("Default");
				OutAnimations.Add(NewAnim);
				DefaultAnim = &OutAnimations.Last();
			}

			if (GroupOrFrameObject->HasField(TEXT("hitboxes")) || GroupOrFrameObject->HasField(TEXT("sockets")))
			{
				FFrameHitboxData FrameData = ParseFrameFromJson(Key, GroupOrFrameObject);
				DefaultAnim->Frames.Add(FrameData);
			}
		}
	}

	return true;
}

EHitboxType FHitboxJsonImporter::ParseHitboxType(const FString& TypeString)
{
	if (TypeString.Equals(TEXT("hurtbox"), ESearchCase::IgnoreCase))
	{
		return EHitboxType::Hurtbox;
	}
	else if (TypeString.Equals(TEXT("collision"), ESearchCase::IgnoreCase))
	{
		return EHitboxType::Collision;
	}
	return EHitboxType::Attack;
}

FHitboxData FHitboxJsonImporter::ParseHitboxFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	FHitboxData Hitbox;
	if (!JsonObject.IsValid()) return Hitbox;

	FString TypeStr;
	if (JsonObject->TryGetStringField(TEXT("type"), TypeStr))
	{
		Hitbox.Type = ParseHitboxType(TypeStr);
	}

	const TArray<TSharedPtr<FJsonValue>>* RectArray;
	if (JsonObject->TryGetArrayField(TEXT("rect"), RectArray) && RectArray->Num() >= 4)
	{
		Hitbox.X = (int32)(*RectArray)[0]->AsNumber();
		Hitbox.Y = (int32)(*RectArray)[1]->AsNumber();
		Hitbox.Width = (int32)(*RectArray)[2]->AsNumber();
		Hitbox.Height = (int32)(*RectArray)[3]->AsNumber();
	}

	Hitbox.Damage = (int32)JsonObject->GetNumberField(TEXT("damage"));
	Hitbox.Knockback = (int32)JsonObject->GetNumberField(TEXT("knockback"));

	return Hitbox;
}

FSocketData FHitboxJsonImporter::ParseSocketFromJson(const TSharedPtr<FJsonObject>& JsonObject)
{
	FSocketData Socket;
	if (!JsonObject.IsValid()) return Socket;

	JsonObject->TryGetStringField(TEXT("name"), Socket.Name);

	const TArray<TSharedPtr<FJsonValue>>* PosArray;
	if (JsonObject->TryGetArrayField(TEXT("pos"), PosArray) && PosArray->Num() >= 2)
	{
		Socket.X = (int32)(*PosArray)[0]->AsNumber();
		Socket.Y = (int32)(*PosArray)[1]->AsNumber();
	}

	return Socket;
}

FFrameHitboxData FHitboxJsonImporter::ParseFrameFromJson(const FString& FrameName, const TSharedPtr<FJsonObject>& JsonObject)
{
	FFrameHitboxData FrameData;
	FrameData.FrameName = FrameName;

	if (!JsonObject.IsValid()) return FrameData;

	const TArray<TSharedPtr<FJsonValue>>* HitboxArray;
	if (JsonObject->TryGetArrayField(TEXT("hitboxes"), HitboxArray))
	{
		for (const TSharedPtr<FJsonValue>& HitboxValue : *HitboxArray)
		{
			if (HitboxValue->Type == EJson::Object)
			{
				FHitboxData Hitbox = ParseHitboxFromJson(HitboxValue->AsObject());
				FrameData.Hitboxes.Add(Hitbox);
			}
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* SocketArray;
	if (JsonObject->TryGetArrayField(TEXT("sockets"), SocketArray))
	{
		for (const TSharedPtr<FJsonValue>& SocketValue : *SocketArray)
		{
			if (SocketValue->Type == EJson::Object)
			{
				FSocketData Socket = ParseSocketFromJson(SocketValue->AsObject());
				FrameData.Sockets.Add(Socket);
			}
		}
	}

	return FrameData;
}

#undef LOCTEXT_NAMESPACE
