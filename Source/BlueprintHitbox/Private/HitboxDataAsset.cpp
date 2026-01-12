#include "HitboxDataAsset.h"

UHitboxDataAsset::UHitboxDataAsset()
{
	DisplayName = TEXT("New Hitbox Data");
}

FPrimaryAssetId UHitboxDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("HitboxData"), GetFName());
}

const FAnimationHitboxData* UHitboxDataAsset::FindAnimation(const FString& AnimationName) const
{
	for (const FAnimationHitboxData& Anim : Animations)
	{
		if (Anim.AnimationName.Equals(AnimationName, ESearchCase::IgnoreCase))
		{
			return &Anim;
		}
	}
	return nullptr;
}

TArray<FString> UHitboxDataAsset::GetAnimationNames() const
{
	TArray<FString> Names;
	Names.Reserve(Animations.Num());
	for (const FAnimationHitboxData& Anim : Animations)
	{
		Names.Add(Anim.AnimationName);
	}
	return Names;
}

bool UHitboxDataAsset::GetAnimation(const FString& AnimationName, FAnimationHitboxData& OutAnimation) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		OutAnimation = *Anim;
		return true;
	}
	return false;
}

bool UHitboxDataAsset::GetAnimationByIndex(int32 Index, FAnimationHitboxData& OutAnimation) const
{
	if (Animations.IsValidIndex(Index))
	{
		OutAnimation = Animations[Index];
		return true;
	}
	return false;
}

int32 UHitboxDataAsset::GetFrameCount(const FString& AnimationName) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		return Anim->Frames.Num();
	}
	return 0;
}

bool UHitboxDataAsset::GetFrame(const FString& AnimationName, int32 FrameIndex, FFrameHitboxData& OutFrame) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		if (const FFrameHitboxData* Frame = Anim->GetFrame(FrameIndex))
		{
			OutFrame = *Frame;
			return true;
		}
	}
	return false;
}

bool UHitboxDataAsset::GetFrameByName(const FString& AnimationName, const FString& FrameName, FFrameHitboxData& OutFrame) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		if (const FFrameHitboxData* Frame = Anim->GetFrameByName(FrameName))
		{
			OutFrame = *Frame;
			return true;
		}
	}
	return false;
}

bool UHitboxDataAsset::FindAnimationByFlipbook(UPaperFlipbook* Flipbook, FAnimationHitboxData& OutAnimation) const
{
	if (!Flipbook) return false;

	for (const FAnimationHitboxData& Anim : Animations)
	{
		if (Anim.Flipbook.Get() == Flipbook)
		{
			OutAnimation = Anim;
			return true;
		}
	}
	return false;
}

TArray<FHitboxData> UHitboxDataAsset::GetHitboxes(const FString& AnimationName, int32 FrameIndex) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		if (const FFrameHitboxData* Frame = Anim->GetFrame(FrameIndex))
		{
			return Frame->Hitboxes;
		}
	}
	return TArray<FHitboxData>();
}

TArray<FHitboxData> UHitboxDataAsset::GetHitboxesByType(const FString& AnimationName, int32 FrameIndex, EHitboxType Type) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		if (const FFrameHitboxData* Frame = Anim->GetFrame(FrameIndex))
		{
			return Frame->GetHitboxesByType(Type);
		}
	}
	return TArray<FHitboxData>();
}

TArray<FSocketData> UHitboxDataAsset::GetSockets(const FString& AnimationName, int32 FrameIndex) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		if (const FFrameHitboxData* Frame = Anim->GetFrame(FrameIndex))
		{
			return Frame->Sockets;
		}
	}
	return TArray<FSocketData>();
}

bool UHitboxDataAsset::FindSocket(const FString& AnimationName, int32 FrameIndex, const FString& SocketName, FSocketData& OutSocket) const
{
	if (const FAnimationHitboxData* Anim = FindAnimation(AnimationName))
	{
		if (const FFrameHitboxData* Frame = Anim->GetFrame(FrameIndex))
		{
			if (const FSocketData* Socket = Frame->FindSocket(SocketName))
			{
				OutSocket = *Socket;
				return true;
			}
		}
	}
	return false;
}

bool UHitboxDataAsset::HasAnimation(const FString& AnimationName) const
{
	return FindAnimation(AnimationName) != nullptr;
}
