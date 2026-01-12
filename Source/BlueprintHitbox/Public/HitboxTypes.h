#pragma once

#include "CoreMinimal.h"
#include "HitboxTypes.generated.h"

/**
 * Type of hitbox - matches the editor export format
 */
UENUM(BlueprintType)
enum class EHitboxType : uint8
{
	Attack		UMETA(DisplayName = "Attack"),
	Hurtbox		UMETA(DisplayName = "Hurtbox"),
	Collision	UMETA(DisplayName = "Collision")
};

/**
 * Single hitbox data
 */
USTRUCT(BlueprintType)
struct BLUEPRINTHITBOX_API FHitboxData
{
	GENERATED_BODY()

	/** Type of hitbox: attack, hurtbox, or collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	EHitboxType Type = EHitboxType::Attack;

	/** X position relative to sprite origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 X = 0;

	/** Y position relative to sprite origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 Y = 0;

	/** Width of the hitbox */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 Width = 0;

	/** Height of the hitbox */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 Height = 0;

	/** Damage dealt (for attack type) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 Damage = 0;

	/** Knockback force (for attack type) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	int32 Knockback = 0;

	/** Get center point */
	FVector2D GetCenter() const
	{
		return FVector2D(X + Width * 0.5f, Y + Height * 0.5f);
	}

	/** Get rect as FBox2D */
	FBox2D GetBox2D() const
	{
		return FBox2D(FVector2D(X, Y), FVector2D(X + Width, Y + Height));
	}
};

/**
 * Socket/attachment point data
 */
USTRUCT(BlueprintType)
struct BLUEPRINTHITBOX_API FSocketData
{
	GENERATED_BODY()

	/** Name of the socket (e.g., "Muzzle", "Hand", "Foot") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FString Name;

	/** X position relative to sprite origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 X = 0;

	/** Y position relative to sprite origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	int32 Y = 0;

	/** Get position as FVector2D */
	FVector2D GetPosition() const
	{
		return FVector2D(X, Y);
	}
};

/**
 * All hitbox/socket data for a single frame
 */
USTRUCT(BlueprintType)
struct BLUEPRINTHITBOX_API FFrameHitboxData
{
	GENERATED_BODY()

	/** Name of the frame (filename without extension) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	FString FrameName;

	/** All hitboxes on this frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	TArray<FHitboxData> Hitboxes;

	/** All sockets/attachment points on this frame */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame")
	TArray<FSocketData> Sockets;

	/** Check if frame has any hitboxes of a specific type */
	bool HasHitboxOfType(EHitboxType Type) const
	{
		for (const FHitboxData& Hitbox : Hitboxes)
		{
			if (Hitbox.Type == Type) return true;
		}
		return false;
	}

	/** Get all hitboxes of a specific type */
	TArray<FHitboxData> GetHitboxesByType(EHitboxType Type) const
	{
		TArray<FHitboxData> Result;
		for (const FHitboxData& Hitbox : Hitboxes)
		{
			if (Hitbox.Type == Type) Result.Add(Hitbox);
		}
		return Result;
	}

	/** Find socket by name */
	const FSocketData* FindSocket(const FString& SocketName) const
	{
		for (const FSocketData& Socket : Sockets)
		{
			if (Socket.Name.Equals(SocketName, ESearchCase::IgnoreCase))
			{
				return &Socket;
			}
		}
		return nullptr;
	}
};

/**
 * Result of a hitbox collision check
 */
USTRUCT(BlueprintType)
struct BLUEPRINTHITBOX_API FHitboxCollisionResult
{
	GENERATED_BODY()

	/** Did a collision occur? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	bool bHit = false;

	/** The attack hitbox that made contact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FHitboxData AttackHitbox;

	/** The hurtbox that was hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FHitboxData HurtHitbox;

	/** World-space center of the collision overlap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FVector2D HitLocation = FVector2D::ZeroVector;

	/** Total damage from this hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	int32 Damage = 0;

	/** Total knockback from this hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	int32 Knockback = 0;
};
