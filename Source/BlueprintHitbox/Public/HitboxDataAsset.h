#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitboxTypes.h"
#include "PaperFlipbook.h"
#include "HitboxDataAsset.generated.h"

/**
 * Animation hitbox data with optional Flipbook reference
 */
USTRUCT(BlueprintType)
struct BLUEPRINTHITBOX_API FAnimationHitboxData
{
	GENERATED_BODY()

	/** Name of the animation/flipbook group */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FString AnimationName;

	/** Optional reference to the Paper2D Flipbook for this animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSoftObjectPtr<UPaperFlipbook> Flipbook;

	/** All frames in this animation with hitbox data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<FFrameHitboxData> Frames;

	/** Get frame data by index */
	const FFrameHitboxData* GetFrame(int32 Index) const
	{
		if (Frames.IsValidIndex(Index))
		{
			return &Frames[Index];
		}
		return nullptr;
	}

	/** Get frame data by name */
	const FFrameHitboxData* GetFrameByName(const FString& FrameName) const
	{
		for (const FFrameHitboxData& Frame : Frames)
		{
			if (Frame.FrameName.Equals(FrameName, ESearchCase::IgnoreCase))
			{
				return &Frame;
			}
		}
		return nullptr;
	}

	/** Get total frame count */
	int32 GetFrameCount() const
	{
		return Frames.Num();
	}
};

/**
 * Data Asset containing all hitbox data for a character or entity.
 * Import from JSON exported by the Hitbox Editor tool.
 */
UCLASS(BlueprintType)
class BLUEPRINTHITBOX_API UHitboxDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UHitboxDataAsset();

	/** Display name for this hitbox data (e.g., character name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox Data")
	FString DisplayName;

	/** All animations with their hitbox data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox Data")
	TArray<FAnimationHitboxData> Animations;

	/** Source JSON file path (for re-importing) */
	UPROPERTY(VisibleAnywhere, Category = "Import")
	FString SourceFilePath;

	/** Last import timestamp */
	UPROPERTY(VisibleAnywhere, Category = "Import")
	FDateTime LastImportTime;

	// ==========================================
	// LOOKUP FUNCTIONS
	// ==========================================

	/** Get all animation names */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	TArray<FString> GetAnimationNames() const;

	/** Get animation data by name */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Data")
	bool GetAnimation(const FString& AnimationName, FAnimationHitboxData& OutAnimation) const;

	/** Get animation data by index */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Data")
	bool GetAnimationByIndex(int32 Index, FAnimationHitboxData& OutAnimation) const;

	/** Get frame count for an animation */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	int32 GetFrameCount(const FString& AnimationName) const;

	/** Get frame data by animation name and frame index */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Data")
	bool GetFrame(const FString& AnimationName, int32 FrameIndex, FFrameHitboxData& OutFrame) const;

	/** Get frame data by animation name and frame name */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Data")
	bool GetFrameByName(const FString& AnimationName, const FString& FrameName, FFrameHitboxData& OutFrame) const;

	/** Find animation index by Flipbook reference */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Data")
	bool FindAnimationByFlipbook(UPaperFlipbook* Flipbook, FAnimationHitboxData& OutAnimation) const;

	// ==========================================
	// DIRECT HITBOX ACCESS
	// ==========================================

	/** Get all hitboxes for a specific frame */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	TArray<FHitboxData> GetHitboxes(const FString& AnimationName, int32 FrameIndex) const;

	/** Get hitboxes of a specific type for a frame */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	TArray<FHitboxData> GetHitboxesByType(const FString& AnimationName, int32 FrameIndex, EHitboxType Type) const;

	/** Get all sockets for a specific frame */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	TArray<FSocketData> GetSockets(const FString& AnimationName, int32 FrameIndex) const;

	/** Find a specific socket by name */
	UFUNCTION(BlueprintCallable, Category = "Hitbox Data")
	bool FindSocket(const FString& AnimationName, int32 FrameIndex, const FString& SocketName, FSocketData& OutSocket) const;

	// ==========================================
	// ASSET INFO
	// ==========================================

	/** Get total number of animations */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	int32 GetAnimationCount() const { return Animations.Num(); }

	/** Check if a specific animation exists */
	UFUNCTION(BlueprintPure, Category = "Hitbox Data")
	bool HasAnimation(const FString& AnimationName) const;

	/** Get asset primary ID for async loading */
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

protected:
	/** Internal lookup - find animation by name */
	const FAnimationHitboxData* FindAnimation(const FString& AnimationName) const;
};
