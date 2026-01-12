#include "HitboxDataAssetEditor.h"
#include "HitboxJsonImporter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "HitboxDataAssetEditor"

// ==========================================
// SHitboxPreviewWidget Implementation
// ==========================================

void SHitboxPreviewWidget::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;
	SelectedAnimationIndex = InArgs._SelectedAnimationIndex;
	SelectedFrameIndex = InArgs._SelectedFrameIndex;
}

FVector2D SHitboxPreviewWidget::ComputeDesiredSize(float) const
{
	return FVector2D(400, 400);
}

int32 SHitboxPreviewWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// Draw background
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(FVector2f(LocalSize), FSlateLayoutTransform()),
		FAppStyle::GetBrush("WhiteBrush"),
		ESlateDrawEffect::None,
		FLinearColor(0.05f, 0.05f, 0.05f, 1.0f)
	);

	const FFrameHitboxData* Frame = GetCurrentFrame();
	if (!Frame)
	{
		return LayerId + 1;
	}

	FVector2D Size = LocalSize;
	float Scale = FMath::Min(Size.X, Size.Y) / 128.0f;
	FVector2D Offset(Size.X * 0.5f - 64 * Scale, Size.Y * 0.5f - 64 * Scale);

	// Draw hitboxes
	for (const FHitboxData& HB : Frame->Hitboxes)
	{
		FLinearColor Color = GetHitboxColor(HB.Type);
		FVector2D Pos = Offset + FVector2D(HB.X, HB.Y) * Scale;
		FVector2D BoxSize(HB.Width * Scale, HB.Height * Scale);

		// Fill
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 1,
			AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize), FSlateLayoutTransform(FVector2f(Pos))),
			FAppStyle::GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			Color * FLinearColor(1, 1, 1, 0.3f)
		);

		// Border
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(FVector2f(BoxSize), FSlateLayoutTransform(FVector2f(Pos))),
			FAppStyle::GetBrush("Border"),
			ESlateDrawEffect::None,
			Color
		);
	}

	// Draw sockets
	for (const FSocketData& Sock : Frame->Sockets)
	{
		FVector2D Pos = Offset + FVector2D(Sock.X, Sock.Y) * Scale;
		float CrossSize = 8.0f;

		TArray<FVector2D> HLine = { FVector2D(Pos.X - CrossSize, Pos.Y), FVector2D(Pos.X + CrossSize, Pos.Y) };
		TArray<FVector2D> VLine = { FVector2D(Pos.X, Pos.Y - CrossSize), FVector2D(Pos.X, Pos.Y + CrossSize) };

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 3,
			AllottedGeometry.ToPaintGeometry(),
			HLine,
			ESlateDrawEffect::None,
			FLinearColor::Yellow,
			true,
			2.0f
		);
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 3,
			AllottedGeometry.ToPaintGeometry(),
			VLine,
			ESlateDrawEffect::None,
			FLinearColor::Yellow,
			true,
			2.0f
		);
	}

	return LayerId + 4;
}

FLinearColor SHitboxPreviewWidget::GetHitboxColor(EHitboxType Type) const
{
	switch (Type)
	{
		case EHitboxType::Attack: return FLinearColor::Red;
		case EHitboxType::Hurtbox: return FLinearColor::Green;
		case EHitboxType::Collision: return FLinearColor::Blue;
		default: return FLinearColor::White;
	}
}

const FFrameHitboxData* SHitboxPreviewWidget::GetCurrentFrame() const
{
	if (!Asset.IsValid()) return nullptr;

	int32 AnimIndex = SelectedAnimationIndex.Get();
	int32 FrameIndex = SelectedFrameIndex.Get();

	if (!Asset->Animations.IsValidIndex(AnimIndex)) return nullptr;

	const FAnimationHitboxData& Anim = Asset->Animations[AnimIndex];
	if (!Anim.Frames.IsValidIndex(FrameIndex)) return nullptr;

	return &Anim.Frames[FrameIndex];
}

// ==========================================
// SHitboxDataAssetEditor Implementation
// ==========================================

void FHitboxDataAssetEditor::OpenEditor(UHitboxDataAsset* Asset)
{
	if (!Asset) return;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::Format(LOCTEXT("WindowTitle", "Hitbox Editor - {0}"), FText::FromString(Asset->DisplayName)))
		.ClientSize(FVector2D(1100, 700))
		.SupportsMinimize(true)
		.SupportsMaximize(true);

	Window->SetContent(
		SNew(SHitboxDataAssetEditor)
		.Asset(Asset)
	);

	FSlateApplication::Get().AddWindow(Window);
}

void SHitboxDataAssetEditor::Construct(const FArguments& InArgs)
{
	Asset = InArgs._Asset;

	ChildSlot
	[
		SNew(SVerticalBox)

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("Reimport", "Reimport from JSON"))
				.OnClicked_Lambda([this]() { OnReimportClicked(); return FReply::Handled(); })
			]
			+ SHorizontalBox::Slot()
			.Padding(20, 0, 0, 0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() -> FText
				{
					if (Asset.IsValid())
						return FText::Format(LOCTEXT("SourceInfo", "Source: {0}"), FText::FromString(Asset->SourceFilePath));
					return LOCTEXT("NoSource", "No source file");
				})
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			]
		]

		// Main content
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(8)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			// Left panel: Animation + Frame lists
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SSplitter)
				.Orientation(Orient_Vertical)

				// Animation list (top)
				+ SSplitter::Slot()
				.Value(0.4f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4)
					[
						BuildAnimationList()
					]
				]

				// Frame list (bottom)
				+ SSplitter::Slot()
				.Value(0.6f)
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4)
					[
						BuildFrameList()
					]
				]
			]

			// Right panel: Preview and details
			+ SSplitter::Slot()
			.Value(0.75f)
			[
				SNew(SBorder)
				.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(4)
				[
					BuildPreviewArea()
				]
			]
		]
	];
}

TSharedRef<SWidget> SHitboxDataAssetEditor::BuildAnimationList()
{
	SAssignNew(AnimationListBox, SVerticalBox);

	AnimationListBox->AddSlot()
	.AutoHeight()
	.Padding(4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Animations", "Animations"))
		.Font(FAppStyle::GetFontStyle("BoldFont"))
	];

	RefreshAnimationList();

	return SNew(SScrollBox) + SScrollBox::Slot()[AnimationListBox.ToSharedRef()];
}

void SHitboxDataAssetEditor::RefreshAnimationList()
{
	if (!AnimationListBox.IsValid()) return;

	// Remove all but the header
	while (AnimationListBox->NumSlots() > 1)
	{
		AnimationListBox->RemoveSlot(AnimationListBox->GetSlot(1).GetWidget());
	}

	if (Asset.IsValid())
	{
		for (int32 i = 0; i < Asset->Animations.Num(); i++)
		{
			const FAnimationHitboxData& Anim = Asset->Animations[i];
			
			bool bIsSelected = (i == SelectedAnimationIndex);
			
			AnimationListBox->AddSlot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(bIsSelected ? FLinearColor(0.2f, 0.4f, 0.8f) : FLinearColor(0.15f, 0.15f, 0.15f))
				.Text(FText::Format(LOCTEXT("AnimBtn", "{0} ({1} frames)"),
					FText::FromString(Anim.AnimationName),
					FText::AsNumber(Anim.Frames.Num())))
				.OnClicked_Lambda([this, i]() { OnAnimationSelected(i); return FReply::Handled(); })
			];
		}
	}
}

TSharedRef<SWidget> SHitboxDataAssetEditor::BuildFrameList()
{
	SAssignNew(FrameListBox, SVerticalBox);

	FrameListBox->AddSlot()
	.AutoHeight()
	.Padding(4)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Frames", "Frames"))
		.Font(FAppStyle::GetFontStyle("BoldFont"))
	];

	RefreshFrameList();

	return SNew(SScrollBox) + SScrollBox::Slot()[FrameListBox.ToSharedRef()];
}

void SHitboxDataAssetEditor::RefreshFrameList()
{
	if (!FrameListBox.IsValid()) return;

	// Remove all but the header
	while (FrameListBox->NumSlots() > 1)
	{
		FrameListBox->RemoveSlot(FrameListBox->GetSlot(1).GetWidget());
	}

	const FAnimationHitboxData* Anim = GetCurrentAnimation();
	if (Anim)
	{
		for (int32 i = 0; i < Anim->Frames.Num(); i++)
		{
			const FFrameHitboxData& Frame = Anim->Frames[i];
			
			bool bIsSelected = (i == SelectedFrameIndex);
			
			// Count hitbox types
			int32 AttackCount = 0, HurtCount = 0, ColCount = 0;
			for (const FHitboxData& HB : Frame.Hitboxes)
			{
				if (HB.Type == EHitboxType::Attack) AttackCount++;
				else if (HB.Type == EHitboxType::Hurtbox) HurtCount++;
				else ColCount++;
			}
			
			FString FrameLabel = FString::Printf(TEXT("%d: %s"), i, *Frame.FrameName);
			FString HitboxInfo = FString::Printf(TEXT("A:%d H:%d C:%d S:%d"), 
				AttackCount, HurtCount, ColCount, Frame.Sockets.Num());
			
			FrameListBox->AddSlot()
			.AutoHeight()
			.Padding(1)
			[
				SNew(SButton)
				.ButtonColorAndOpacity(bIsSelected ? FLinearColor(0.2f, 0.6f, 0.3f) : FLinearColor(0.12f, 0.12f, 0.12f))
				.OnClicked_Lambda([this, i]() { OnFrameSelected(i); return FReply::Handled(); })
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(FrameLabel))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(HitboxInfo))
						.Font(FAppStyle::GetFontStyle("SmallFont"))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
					]
				]
			];
		}
	}
}

TSharedRef<SWidget> SHitboxDataAssetEditor::BuildFrameNavigation()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("PrevFrame", "< Prev"))
			.OnClicked_Lambda([this]() { OnPrevFrameClicked(); return FReply::Handled(); })
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() -> FText
			{
				int32 FrameCount = GetCurrentFrameCount();
				if (FrameCount > 0)
				{
					return FText::Format(LOCTEXT("FrameCounter", "Frame {0} / {1}"),
						FText::AsNumber(SelectedFrameIndex + 1),
						FText::AsNumber(FrameCount));
				}
				return LOCTEXT("NoFrames", "No frames");
			})
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("NextFrame", "Next >"))
			.OnClicked_Lambda([this]() { OnNextFrameClicked(); return FReply::Handled(); })
		];
}

TSharedRef<SWidget> SHitboxDataAssetEditor::BuildPreviewArea()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Preview", "Preview"))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		// Frame navigation
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4)
		[
			BuildFrameNavigation()
		]
		// Preview canvas
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(4)
		[
			SNew(SBox)
			.MinDesiredWidth(400)
			.MinDesiredHeight(300)
			[
				SNew(SHitboxPreviewWidget)
				.Asset(Asset)
				.SelectedAnimationIndex_Lambda([this]() { return SelectedAnimationIndex; })
				.SelectedFrameIndex_Lambda([this]() { return SelectedFrameIndex; })
			]
		]
		// Details panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(200)
		.Padding(4)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				BuildDetailsPanel()
			]
		];
}

TSharedRef<SWidget> SHitboxDataAssetEditor::BuildDetailsPanel()
{
	return SNew(STextBlock)
		.Text_Lambda([this]() -> FText
		{
			const FFrameHitboxData* Frame = GetCurrentFrame();
			if (!Frame) return LOCTEXT("SelectFrame", "Select an animation and frame to view details");

			FString Info = FString::Printf(TEXT("Frame: %s\nHitboxes: %d | Sockets: %d\n"),
				*Frame->FrameName, Frame->Hitboxes.Num(), Frame->Sockets.Num());

			Info += TEXT("\n--- Hitboxes ---\n");
			for (int32 i = 0; i < Frame->Hitboxes.Num(); i++)
			{
				const FHitboxData& HB = Frame->Hitboxes[i];
				FString Type = HB.Type == EHitboxType::Attack ? TEXT("ATTACK") :
							   HB.Type == EHitboxType::Hurtbox ? TEXT("HURTBOX") : TEXT("COLLISION");
				Info += FString::Printf(TEXT("[%d] %s: Pos(%d,%d) Size(%dx%d) Dmg:%d KB:%d\n"),
					i, *Type, HB.X, HB.Y, HB.Width, HB.Height, HB.Damage, HB.Knockback);
			}

			if (Frame->Sockets.Num() > 0)
			{
				Info += TEXT("\n--- Sockets ---\n");
				for (const FSocketData& Sock : Frame->Sockets)
				{
					Info += FString::Printf(TEXT("%s: (%d, %d)\n"), *Sock.Name, Sock.X, Sock.Y);
				}
			}

			return FText::FromString(Info);
		});
}

void SHitboxDataAssetEditor::OnAnimationSelected(int32 Index)
{
	SelectedAnimationIndex = Index;
	SelectedFrameIndex = 0;
	RefreshAnimationList();
	RefreshFrameList();
}

void SHitboxDataAssetEditor::OnFrameSelected(int32 Index)
{
	SelectedFrameIndex = Index;
	RefreshFrameList();
}

void SHitboxDataAssetEditor::OnPrevFrameClicked()
{
	if (SelectedFrameIndex > 0)
	{
		SelectedFrameIndex--;
		RefreshFrameList();
	}
}

void SHitboxDataAssetEditor::OnNextFrameClicked()
{
	int32 FrameCount = GetCurrentFrameCount();
	if (SelectedFrameIndex < FrameCount - 1)
	{
		SelectedFrameIndex++;
		RefreshFrameList();
	}
}

void SHitboxDataAssetEditor::OnReimportClicked()
{
	if (Asset.IsValid())
	{
		FString ErrorMessage;
		if (FHitboxJsonImporter::ReimportAsset(Asset.Get(), ErrorMessage))
		{
			// Refresh lists after reimport
			SelectedAnimationIndex = 0;
			SelectedFrameIndex = 0;
			RefreshAnimationList();
			RefreshFrameList();
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ReimportSuccess", "Reimport successful!"));
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok,
				FText::Format(LOCTEXT("ReimportFailed", "Reimport failed: {0}"), FText::FromString(ErrorMessage)));
		}
	}
}

const FFrameHitboxData* SHitboxDataAssetEditor::GetCurrentFrame() const
{
	const FAnimationHitboxData* Anim = GetCurrentAnimation();
	if (!Anim) return nullptr;
	if (!Anim->Frames.IsValidIndex(SelectedFrameIndex)) return nullptr;
	return &Anim->Frames[SelectedFrameIndex];
}

const FAnimationHitboxData* SHitboxDataAssetEditor::GetCurrentAnimation() const
{
	if (!Asset.IsValid()) return nullptr;
	if (!Asset->Animations.IsValidIndex(SelectedAnimationIndex)) return nullptr;
	return &Asset->Animations[SelectedAnimationIndex];
}

int32 SHitboxDataAssetEditor::GetCurrentFrameCount() const
{
	const FAnimationHitboxData* Anim = GetCurrentAnimation();
	return Anim ? Anim->Frames.Num() : 0;
}

#undef LOCTEXT_NAMESPACE
