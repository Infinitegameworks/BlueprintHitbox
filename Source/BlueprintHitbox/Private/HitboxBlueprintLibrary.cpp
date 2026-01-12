#include "HitboxBlueprintLibrary.h"

// ==========================================
// WORLD SPACE CONVERSION
// ==========================================

FBox2D UHitboxBlueprintLibrary::HitboxToWorldSpace(const FHitboxData& Hitbox, FVector2D WorldPosition, bool bFlipX, float Scale)
{
	float X = Hitbox.X * Scale;
	float Y = Hitbox.Y * Scale;
	float W = Hitbox.Width * Scale;
	float H = Hitbox.Height * Scale;

	if (bFlipX)
	{
		// Flip horizontally around origin
		X = -(X + W);
	}

	return FBox2D(
		FVector2D(WorldPosition.X + X, WorldPosition.Y + Y),
		FVector2D(WorldPosition.X + X + W, WorldPosition.Y + Y + H)
	);
}

FBox2D UHitboxBlueprintLibrary::HitboxToWorldSpace3D(const FHitboxData& Hitbox, FVector WorldPosition, bool bFlipX, float Scale)
{
	// Use X for horizontal, Z for vertical (common 2D-in-3D setup)
	return HitboxToWorldSpace(Hitbox, FVector2D(WorldPosition.X, WorldPosition.Z), bFlipX, Scale);
}

FVector2D UHitboxBlueprintLibrary::SocketToWorldSpace(const FSocketData& Socket, FVector2D WorldPosition, bool bFlipX, float Scale)
{
	float X = Socket.X * Scale;
	float Y = Socket.Y * Scale;

	if (bFlipX)
	{
		X = -X;
	}

	return FVector2D(WorldPosition.X + X, WorldPosition.Y + Y);
}

FVector UHitboxBlueprintLibrary::SocketToWorldSpace3D(const FSocketData& Socket, FVector WorldPosition, bool bFlipX, float Scale)
{
	FVector2D Pos2D = SocketToWorldSpace(Socket, FVector2D(WorldPosition.X, WorldPosition.Z), bFlipX, Scale);
	return FVector(Pos2D.X, WorldPosition.Y, Pos2D.Y);
}

// ==========================================
// COLLISION DETECTION
// ==========================================

bool UHitboxBlueprintLibrary::DoBoxesOverlap(const FBox2D& BoxA, const FBox2D& BoxB)
{
	return BoxA.Intersect(BoxB);
}

bool UHitboxBlueprintLibrary::CheckHitboxCollision(
	const FFrameHitboxData& AttackerFrame,
	FVector2D AttackerPosition,
	bool bAttackerFlipX,
	float AttackerScale,
	const FFrameHitboxData& DefenderFrame,
	FVector2D DefenderPosition,
	bool bDefenderFlipX,
	float DefenderScale,
	TArray<FHitboxCollisionResult>& OutResults)
{
	OutResults.Empty();

	// Get attack hitboxes from attacker
	TArray<FHitboxData> AttackBoxes = AttackerFrame.GetHitboxesByType(EHitboxType::Attack);
	if (AttackBoxes.Num() == 0) return false;

	// Get hurtboxes from defender
	TArray<FHitboxData> HurtBoxes = DefenderFrame.GetHitboxesByType(EHitboxType::Hurtbox);
	if (HurtBoxes.Num() == 0) return false;

	bool bAnyHit = false;

	// Check each attack box against each hurtbox
	for (const FHitboxData& Attack : AttackBoxes)
	{
		FBox2D AttackWorld = HitboxToWorldSpace(Attack, AttackerPosition, bAttackerFlipX, AttackerScale);

		for (const FHitboxData& Hurt : HurtBoxes)
		{
			FBox2D HurtWorld = HitboxToWorldSpace(Hurt, DefenderPosition, bDefenderFlipX, DefenderScale);

			if (AttackWorld.Intersect(HurtWorld))
			{
				FHitboxCollisionResult Result;
				Result.bHit = true;
				Result.AttackHitbox = Attack;
				Result.HurtHitbox = Hurt;
				Result.Damage = Attack.Damage;
				Result.Knockback = Attack.Knockback;

				// Calculate hit location (center of overlap)
				FBox2D Overlap(
					FVector2D(FMath::Max(AttackWorld.Min.X, HurtWorld.Min.X), FMath::Max(AttackWorld.Min.Y, HurtWorld.Min.Y)),
					FVector2D(FMath::Min(AttackWorld.Max.X, HurtWorld.Max.X), FMath::Min(AttackWorld.Max.Y, HurtWorld.Max.Y))
				);
				Result.HitLocation = Overlap.GetCenter();

				OutResults.Add(Result);
				bAnyHit = true;
			}
		}
	}

	return bAnyHit;
}

bool UHitboxBlueprintLibrary::CheckHitboxCollision3D(
	const FFrameHitboxData& AttackerFrame,
	FVector AttackerPosition,
	bool bAttackerFlipX,
	float AttackerScale,
	const FFrameHitboxData& DefenderFrame,
	FVector DefenderPosition,
	bool bDefenderFlipX,
	float DefenderScale,
	TArray<FHitboxCollisionResult>& OutResults)
{
	return CheckHitboxCollision(
		AttackerFrame,
		FVector2D(AttackerPosition.X, AttackerPosition.Z),
		bAttackerFlipX,
		AttackerScale,
		DefenderFrame,
		FVector2D(DefenderPosition.X, DefenderPosition.Z),
		bDefenderFlipX,
		DefenderScale,
		OutResults
	);
}

bool UHitboxBlueprintLibrary::QuickHitCheck(
	const FFrameHitboxData& AttackerFrame,
	FVector2D AttackerPosition,
	bool bAttackerFlipX,
	float AttackerScale,
	const FFrameHitboxData& DefenderFrame,
	FVector2D DefenderPosition,
	bool bDefenderFlipX,
	float DefenderScale)
{
	TArray<FHitboxData> AttackBoxes = AttackerFrame.GetHitboxesByType(EHitboxType::Attack);
	if (AttackBoxes.Num() == 0) return false;

	TArray<FHitboxData> HurtBoxes = DefenderFrame.GetHitboxesByType(EHitboxType::Hurtbox);
	if (HurtBoxes.Num() == 0) return false;

	for (const FHitboxData& Attack : AttackBoxes)
	{
		FBox2D AttackWorld = HitboxToWorldSpace(Attack, AttackerPosition, bAttackerFlipX, AttackerScale);

		for (const FHitboxData& Hurt : HurtBoxes)
		{
			FBox2D HurtWorld = HitboxToWorldSpace(Hurt, DefenderPosition, bDefenderFlipX, DefenderScale);

			if (AttackWorld.Intersect(HurtWorld))
			{
				return true;
			}
		}
	}

	return false;
}

// ==========================================
// FRAME DATA HELPERS
// ==========================================

TArray<FHitboxData> UHitboxBlueprintLibrary::GetAttackHitboxes(const FFrameHitboxData& FrameData)
{
	return FrameData.GetHitboxesByType(EHitboxType::Attack);
}

TArray<FHitboxData> UHitboxBlueprintLibrary::GetHurtboxes(const FFrameHitboxData& FrameData)
{
	return FrameData.GetHitboxesByType(EHitboxType::Hurtbox);
}

TArray<FHitboxData> UHitboxBlueprintLibrary::GetCollisionBoxes(const FFrameHitboxData& FrameData)
{
	return FrameData.GetHitboxesByType(EHitboxType::Collision);
}

bool UHitboxBlueprintLibrary::HasAttackHitboxes(const FFrameHitboxData& FrameData)
{
	return FrameData.HasHitboxOfType(EHitboxType::Attack);
}

bool UHitboxBlueprintLibrary::HasHurtboxes(const FFrameHitboxData& FrameData)
{
	return FrameData.HasHitboxOfType(EHitboxType::Hurtbox);
}

bool UHitboxBlueprintLibrary::HasAnyData(const FFrameHitboxData& FrameData)
{
	return FrameData.Hitboxes.Num() > 0 || FrameData.Sockets.Num() > 0;
}

// ==========================================
// UTILITIES
// ==========================================

FString UHitboxBlueprintLibrary::HitboxTypeToString(EHitboxType Type)
{
	switch (Type)
	{
	case EHitboxType::Attack:
		return TEXT("Attack");
	case EHitboxType::Hurtbox:
		return TEXT("Hurtbox");
	case EHitboxType::Collision:
		return TEXT("Collision");
	default:
		return TEXT("Unknown");
	}
}

EHitboxType UHitboxBlueprintLibrary::StringToHitboxType(const FString& TypeString)
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

FVector2D UHitboxBlueprintLibrary::GetBoxCenter(const FBox2D& Box)
{
	return Box.GetCenter();
}

FVector2D UHitboxBlueprintLibrary::GetBoxSize(const FBox2D& Box)
{
	return Box.GetSize();
}

FBox2D UHitboxBlueprintLibrary::MakeBox2D(FVector2D Center, FVector2D HalfExtents)
{
	return FBox2D(Center - HalfExtents, Center + HalfExtents);
}

int32 UHitboxBlueprintLibrary::GetTotalDamage(const TArray<FHitboxCollisionResult>& Results)
{
	int32 Total = 0;
	for (const FHitboxCollisionResult& Result : Results)
	{
		Total += Result.Damage;
	}
	return Total;
}

int32 UHitboxBlueprintLibrary::GetMaxKnockback(const TArray<FHitboxCollisionResult>& Results)
{
	int32 Max = 0;
	for (const FHitboxCollisionResult& Result : Results)
	{
		if (Result.Knockback > Max)
		{
			Max = Result.Knockback;
		}
	}
	return Max;
}
