#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "HitboxDataAsset.h"

class UPaperSprite;
class SVerticalBox;

/**
 * Custom widget for rendering hitbox preview (replaces deprecated OnPaint_Raw)
 */
class SHitboxPreviewWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SHitboxPreviewWidget) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UHitboxDataAsset>, Asset)
		SLATE_ATTRIBUTE(int32, SelectedAnimationIndex)
		SLATE_ATTRIBUTE(int32, SelectedFrameIndex)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	TWeakObjectPtr<UHitboxDataAsset> Asset;
	TAttribute<int32> SelectedAnimationIndex;
	TAttribute<int32> SelectedFrameIndex;

	FLinearColor GetHitboxColor(EHitboxType Type) const;
	const FFrameHitboxData* GetCurrentFrame() const;
};

/**
 * Custom editor window for HitboxDataAsset.
 * Shows animation list, frame list, and hitbox visualization preview.
 */
class SHitboxDataAssetEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHitboxDataAssetEditor) {}
		SLATE_ARGUMENT(UHitboxDataAsset*, Asset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// The asset being edited
	TWeakObjectPtr<UHitboxDataAsset> Asset;

	// Currently selected animation and frame
	int32 SelectedAnimationIndex = 0;
	int32 SelectedFrameIndex = 0;

	// Cached widget references for dynamic updates
	TSharedPtr<SVerticalBox> FrameListBox;
	TSharedPtr<SVerticalBox> AnimationListBox;

	// UI Builders
	TSharedRef<SWidget> BuildAnimationList();
	TSharedRef<SWidget> BuildFrameList();
	TSharedRef<SWidget> BuildPreviewArea();
	TSharedRef<SWidget> BuildDetailsPanel();
	TSharedRef<SWidget> BuildFrameNavigation();

	// Refresh functions
	void RefreshFrameList();
	void RefreshAnimationList();

	// Event handlers
	void OnAnimationSelected(int32 Index);
	void OnFrameSelected(int32 Index);
	void OnReimportClicked();
	void OnPrevFrameClicked();
	void OnNextFrameClicked();

	// Helpers
	const FFrameHitboxData* GetCurrentFrame() const;
	const FAnimationHitboxData* GetCurrentAnimation() const;
	int32 GetCurrentFrameCount() const;
};

/**
 * Static class to manage opening the editor window
 */
class FHitboxDataAssetEditor
{
public:
	static void OpenEditor(UHitboxDataAsset* Asset);
};
