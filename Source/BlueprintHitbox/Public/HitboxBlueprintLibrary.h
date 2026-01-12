#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HitboxTypes.h"
#include "HitboxDataAsset.h"
#include "HitboxBlueprintLibrary.generated.h"

/**
 * Blueprint function library for hitbox operations.
 * Provides world-space conversion, collision detection, and utility functions.
 */
UCLASS()
class BLUEPRINTHITBOX_API UHitboxBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==========================================
	// WORLD SPACE CONVERSION
	// ==========================================

	/**
	 * Convert a hitbox to world space Box2D
	 * @param Hitbox The hitbox data
	 * @param WorldPosition Character's world position (2D)
	 * @param bFlipX Whether character is facing left (flips hitbox horizontally)
	 * @param Scale Scale multiplier (default 1.0)
	 * @return World-space Box2D
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Conversion")
	static FBox2D HitboxToWorldSpace(const FHitboxData& Hitbox, FVector2D WorldPosition, bool bFlipX, float Scale = 1.0f);

	/**
	 * Convert a hitbox to world space with 3D vector position (uses X and Z for 2D)
	 * @param Hitbox The hitbox data  
	 * @param WorldPosition Character's world position (3D, uses X and Z)
	 * @param bFlipX Whether character is facing left
	 * @param Scale Scale multiplier
	 * @return World-space Box2D
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Conversion")
	static FBox2D HitboxToWorldSpace3D(const FHitboxData& Hitbox, FVector WorldPosition, bool bFlipX, float Scale = 1.0f);

	/**
	 * Convert socket position to world space
	 * @param Socket The socket data
	 * @param WorldPosition Character's world position (2D)
	 * @param bFlipX Whether character is facing left
	 * @param Scale Scale multiplier
	 * @return World-space 2D position
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Conversion")
	static FVector2D SocketToWorldSpace(const FSocketData& Socket, FVector2D WorldPosition, bool bFlipX, float Scale = 1.0f);

	/**
	 * Convert socket to world space with 3D vector (uses X and Z for 2D)
	 * @param Socket The socket data
	 * @param WorldPosition Character's world position (3D)
	 * @param bFlipX Whether character is facing left
	 * @param Scale Scale multiplier
	 * @return World-space 3D position (Y = 0)
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Conversion")
	static FVector SocketToWorldSpace3D(const FSocketData& Socket, FVector WorldPosition, bool bFlipX, float Scale = 1.0f);

	// ==========================================
	// COLLISION DETECTION
	// ==========================================

	/**
	 * Check if two Box2D overlap
	 * @param BoxA First box
	 * @param BoxB Second box
	 * @return True if overlapping
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Collision")
	static bool DoBoxesOverlap(const FBox2D& BoxA, const FBox2D& BoxB);

	/**
	 * Check collision between attacker and defender hitboxes (single frame)
	 * Checks all attack hitboxes against all hurtboxes
	 * @param AttackerFrame Attacker's current frame data
	 * @param AttackerPosition Attacker's world position
	 * @param bAttackerFlipX Is attacker facing left
	 * @param AttackerScale Attacker's scale
	 * @param DefenderFrame Defender's current frame data
	 * @param DefenderPosition Defender's world position
	 * @param bDefenderFlipX Is defender facing left
	 * @param DefenderScale Defender's scale
	 * @param OutResults Array of collision results (one per hit)
	 * @return True if any collision occurred
	 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Collision")
	static bool CheckHitboxCollision(
		const FFrameHitboxData& AttackerFrame,
		FVector2D AttackerPosition,
		bool bAttackerFlipX,
		float AttackerScale,
		const FFrameHitboxData& DefenderFrame,
		FVector2D DefenderPosition,
		bool bDefenderFlipX,
		float DefenderScale,
		TArray<FHitboxCollisionResult>& OutResults
	);

	/**
	 * Check collision using 3D positions (uses X and Z)
	 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Collision")
	static bool CheckHitboxCollision3D(
		const FFrameHitboxData& AttackerFrame,
		FVector AttackerPosition,
		bool bAttackerFlipX,
		float AttackerScale,
		const FFrameHitboxData& DefenderFrame,
		FVector DefenderPosition,
		bool bDefenderFlipX,
		float DefenderScale,
		TArray<FHitboxCollisionResult>& OutResults
	);

	/**
	 * Quick check if any attack hitbox overlaps any hurtbox (no detailed results)
	 * Faster than full collision check when you only need a boolean
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Collision")
	static bool QuickHitCheck(
		const FFrameHitboxData& AttackerFrame,
		FVector2D AttackerPosition,
		bool bAttackerFlipX,
		float AttackerScale,
		const FFrameHitboxData& DefenderFrame,
		FVector2D DefenderPosition,
		bool bDefenderFlipX,
		float DefenderScale
	);

	// ==========================================
	// FRAME DATA HELPERS
	// ==========================================

	/**
	 * Get all attack hitboxes from a frame
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Frame")
	static TArray<FHitboxData> GetAttackHitboxes(const FFrameHitboxData& FrameData);

	/**
	 * Get all hurtboxes from a frame
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Frame")
	static TArray<FHitboxData> GetHurtboxes(const FFrameHitboxData& FrameData);

	/**
	 * Get all collision boxes from a frame
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Frame")
	static TArray<FHitboxData> GetCollisionBoxes(const FFrameHitboxData& FrameData);

	/**
	 * Check if frame has any attack hitboxes
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Frame")
	static bool HasAttackHitboxes(const FFrameHitboxData& FrameData);

	/**
	 * Check if frame has any hurtboxes
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Frame")
	static bool HasHurtboxes(const FFrameHitboxData& FrameData);

	/**
	 * Check if frame has any data at all
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Frame")
	static bool HasAnyData(const FFrameHitboxData& FrameData);

	// ==========================================
	// UTILITIES
	// ==========================================

	/**
	 * Get hitbox type as display string
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static FString HitboxTypeToString(EHitboxType Type);

	/**
	 * Parse hitbox type from string
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static EHitboxType StringToHitboxType(const FString& TypeString);

	/**
	 * Get the center point of a Box2D
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static FVector2D GetBoxCenter(const FBox2D& Box);

	/**
	 * Get the size (extent) of a Box2D
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static FVector2D GetBoxSize(const FBox2D& Box);

	/**
	 * Create a Box2D from center and half-extents
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static FBox2D MakeBox2D(FVector2D Center, FVector2D HalfExtents);

	/**
	 * Calculate total damage from collision results
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static int32 GetTotalDamage(const TArray<FHitboxCollisionResult>& Results);

	/**
	 * Calculate highest knockback from collision results
	 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Utilities")
	static int32 GetMaxKnockback(const TArray<FHitboxCollisionResult>& Results);
};
