// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/STalentTreeWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

void STalentTreeWidget::Construct(const FArguments& InArgs)
{
	RingSpacing         = InArgs._RingSpacing;
	NodeRadius          = InArgs._NodeRadius;
	TooltipDelaySeconds = InArgs._TooltipDelaySeconds;
	LockedColor         = InArgs._LockedColor;
	AvailableColor      = InArgs._AvailableColor;
	PartialColor        = InArgs._PartialColor;
	FullColor           = InArgs._FullColor;
	LinkColor           = InArgs._LinkColor;
	RingColor           = InArgs._RingColor;

	OnGetNodePointsDelegate     = InArgs._OnGetNodePoints;
	OnGetAvailablePointsDelegate= InArgs._OnGetAvailablePoints;
	OnIsUnlockedDelegate        = InArgs._OnIsUnlocked;
	OnInvestDelegate            = InArgs._OnInvest;

	// A white rounded-box brush; every node disc is drawn with this and tinted per state.
	NodeBackgroundBrush = FSlateRoundedBoxBrush(FLinearColor::White, NodeRadius, FVector2D(NodeRadius * 2.f, NodeRadius * 2.f));

	NodeFont  = FCoreStyle::GetDefaultFontStyle("Bold", 9);
	CountFont = FCoreStyle::GetDefaultFontStyle("Bold", 8);

	SetCanTick(true);

	// The tooltip lives in a hit-test-invisible overlay so it never steals mouse input from the tree.
	ChildSlot
	[
		SNew(SOverlay)
		.Visibility(EVisibility::HitTestInvisible)
		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		[
			SAssignNew(TooltipContainer, SBox)
			.Visibility(EVisibility::Collapsed)
			.MaxDesiredWidth(320.f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(8.f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.f, 0.f, 6.f, 0.f)
						[
							SAssignNew(TooltipIcon, SImage)
							.Image(&TooltipIconBrush)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SAssignNew(TooltipTitle, STextBlock)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(TooltipBody, STextBlock)
						.AutoWrapText(true)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 4.f, 0.f, 0.f)
					[
						SAssignNew(TooltipPoints, STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.9f, 1.f, 1.f)))
					]
				]
			]
		]
	];
}

void STalentTreeWidget::SetNodes(TArray<FTalentTreeSlateNode> InNodes)
{
	Nodes = MoveTemp(InNodes);
	IdToIndex.Reset();
	MaxRing = 0;
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		IdToIndex.Add(Nodes[i].Id, i);
		MaxRing = FMath::Max(MaxRing, Nodes[i].Ring);
	}
	HoveredNodeIndex = INDEX_NONE;
	HoverElapsed = 0.f;
	if (TooltipContainer.IsValid())
	{
		TooltipContainer->SetVisibility(EVisibility::Collapsed);
	}
	Invalidate(EInvalidateWidgetReason::Layout);
}

FVector2D STalentTreeWidget::GetNodeLocalCenter(const FTalentTreeSlateNode& Node, const FVector2D& LocalSize) const
{
	const FVector2D Center = LocalSize * 0.5f;
	const float R = (float)Node.Ring * RingSpacing;
	const float A = FMath::DegreesToRadians(Node.Angle);
	// Angle 0 = up (12 o'clock), increasing clockwise.
	return Center + FVector2D(FMath::Sin(A), -FMath::Cos(A)) * R;
}

int32 STalentTreeWidget::HitTestNode(const FVector2D& LocalPos, const FVector2D& LocalSize) const
{
	const float HitRadiusSq = FMath::Square(NodeRadius + 2.f);
	for (int32 i = 0; i < Nodes.Num(); ++i)
	{
		const FVector2D C = GetNodeLocalCenter(Nodes[i], LocalSize);
		if (FVector2D::DistSquared(LocalPos, C) <= HitRadiusSq)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 STalentTreeWidget::GetNodePoints(FName Id) const
{
	return OnGetNodePointsDelegate.IsBound() ? OnGetNodePointsDelegate.Execute(Id) : 0;
}

bool STalentTreeWidget::IsUnlocked(FName Id) const
{
	return OnIsUnlockedDelegate.IsBound() ? OnIsUnlockedDelegate.Execute(Id) : true;
}

FVector2D STalentTreeWidget::ComputeDesiredSize(float) const
{
	const float R = (float)FMath::Max(MaxRing, 1) * RingSpacing + NodeRadius + 12.f;
	return FVector2D(R * 2.f, R * 2.f);
}

int32 STalentTreeWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FVector2D LocalSize = AllottedGeometry.GetLocalSize();
	const FVector2D Center = LocalSize * 0.5f;
	const FPaintGeometry WholeGeom = AllottedGeometry.ToPaintGeometry();

	int32 Layer = LayerId;

	// --- 0. Empty-state placeholder ---
	// If there are no nodes (usually: no TalentTreeDataTable assigned to the unit's WeaponComponent),
	// still draw a visible ring + centre disc + hint text so it's obvious the widget IS alive.
	if (Nodes.Num() == 0)
	{
		const int32 Segments = 72;
		const float R = RingSpacing;
		TArray<FVector2D> Points;
		Points.Reserve(Segments + 1);
		for (int32 s = 0; s <= Segments; ++s)
		{
			const float A = (2.f * PI * (float)s) / (float)Segments;
			Points.Add(Center + FVector2D(FMath::Sin(A), -FMath::Cos(A)) * R);
		}
		FSlateDrawElement::MakeLines(OutDrawElements, Layer, WholeGeom, Points, ESlateDrawEffect::None, AvailableColor, true, 2.f);

		const float Size = NodeRadius * 2.f;
		const FVector2D TopLeft = Center - FVector2D(NodeRadius, NodeRadius);
		FSlateDrawElement::MakeBox(OutDrawElements, Layer + 1,
			AllottedGeometry.ToPaintGeometry(FVector2D(Size, Size), FSlateLayoutTransform(1.f, TopLeft)),
			&NodeBackgroundBrush, ESlateDrawEffect::None, LockedColor);

		const TSharedRef<FSlateFontMeasure> FM = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FString Msg = TEXT("TalentTree: kein DataTable zugewiesen");
		const FVector2D TextSize = FM->Measure(Msg, NodeFont);
		FSlateDrawElement::MakeText(OutDrawElements, Layer + 2,
			AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(1.f, Center + FVector2D(-TextSize.X * 0.5f, NodeRadius + 6.f))),
			Msg, NodeFont, ESlateDrawEffect::None, FLinearColor::White);

		return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, Layer + 3, InWidgetStyle, bParentEnabled);
	}

	// --- 1. Faint ring guide circles ---
	{
		const int32 Segments = 72;
		for (int32 Ring = 1; Ring <= MaxRing; ++Ring)
		{
			const float R = (float)Ring * RingSpacing;
			TArray<FVector2D> Points;
			Points.Reserve(Segments + 1);
			for (int32 s = 0; s <= Segments; ++s)
			{
				const float A = (2.f * PI * (float)s) / (float)Segments;
				Points.Add(Center + FVector2D(FMath::Sin(A), -FMath::Cos(A)) * R);
			}
			FSlateDrawElement::MakeLines(OutDrawElements, Layer, WholeGeom, Points, ESlateDrawEffect::None, RingColor, true, 1.5f);
		}
	}
	++Layer;

	// --- 2. Links (child -> parent) ---
	{
		for (const FTalentTreeSlateNode& Node : Nodes)
		{
			if (Node.PrevId.IsNone())
			{
				continue;
			}
			const int32* ParentIdx = IdToIndex.Find(Node.PrevId);
			if (!ParentIdx || !Nodes.IsValidIndex(*ParentIdx))
			{
				continue;
			}
			const FVector2D ChildC = GetNodeLocalCenter(Node, LocalSize);
			const FVector2D ParentC = GetNodeLocalCenter(Nodes[*ParentIdx], LocalSize);

			const int32 Pts = GetNodePoints(Node.Id);
			FLinearColor C = LinkColor;
			if (Pts >= FMath::Max(1, Node.MaxPoints)) { C = FullColor; }
			else if (IsUnlocked(Node.Id))            { C = LinkColor * 1.8f; C.A = 1.f; }

			TArray<FVector2D> Line;
			Line.Add(ParentC);
			Line.Add(ChildC);
			FSlateDrawElement::MakeLines(OutDrawElements, Layer, WholeGeom, Line, ESlateDrawEffect::None, C, true, 3.f);
		}
	}
	++Layer;

	// --- 3. Nodes ---
	const int32 NodeLayer = Layer;
	const int32 IconLayer = Layer + 1;
	const int32 TextLayer = Layer + 2;
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	for (const FTalentTreeSlateNode& Node : Nodes)
	{
		const FVector2D C = GetNodeLocalCenter(Node, LocalSize);
		const int32 Pts = GetNodePoints(Node.Id);
		const int32 MaxPts = FMath::Max(1, Node.MaxPoints);
		const bool bUnlocked = IsUnlocked(Node.Id);

		FLinearColor StateColor;
		if (!bUnlocked)            StateColor = LockedColor;
		else if (Pts <= 0)         StateColor = AvailableColor;
		else if (Pts >= MaxPts)    StateColor = FullColor;
		else                       StateColor = PartialColor;

		// Outer rim (branch identity colour).
		{
			const float Size = NodeRadius * 2.f;
			const FVector2D TopLeft = C - FVector2D(NodeRadius, NodeRadius);
			FSlateDrawElement::MakeBox(OutDrawElements, NodeLayer,
				AllottedGeometry.ToPaintGeometry(FVector2D(Size, Size), FSlateLayoutTransform(1.f, TopLeft)),
				&NodeBackgroundBrush, ESlateDrawEffect::None, Node.BaseColor);
		}
		// Inner disc (state colour).
		{
			const float Inner = FMath::Max(4.f, NodeRadius - 3.f);
			const float Size = Inner * 2.f;
			const FVector2D TopLeft = C - FVector2D(Inner, Inner);
			FSlateDrawElement::MakeBox(OutDrawElements, NodeLayer,
				AllottedGeometry.ToPaintGeometry(FVector2D(Size, Size), FSlateLayoutTransform(1.f, TopLeft)),
				&NodeBackgroundBrush, ESlateDrawEffect::None, StateColor);
		}

		// Icon (dim when locked).
		if (Node.bHasIcon)
		{
			const float IconSize = FMath::Max(6.f, (NodeRadius - 6.f) * 2.f);
			const FVector2D TopLeft = C - FVector2D(IconSize * 0.5f, IconSize * 0.5f);
			const FLinearColor IconTint = bUnlocked ? FLinearColor::White : FLinearColor(0.5f, 0.5f, 0.5f, 0.7f);
			FSlateDrawElement::MakeBox(OutDrawElements, IconLayer,
				AllottedGeometry.ToPaintGeometry(FVector2D(IconSize, IconSize), FSlateLayoutTransform(1.f, TopLeft)),
				&Node.IconBrush, ESlateDrawEffect::None, IconTint);
		}

		// Points count "x/max".
		{
			const FString CountStr = FString::Printf(TEXT("%d/%d"), Pts, MaxPts);
			const FVector2D TextSize = FontMeasure->Measure(CountStr, CountFont);
			const FVector2D TextPos = C + FVector2D(-TextSize.X * 0.5f, NodeRadius - 2.f);
			FSlateDrawElement::MakeText(OutDrawElements, TextLayer,
				AllottedGeometry.ToPaintGeometry(TextSize, FSlateLayoutTransform(1.f, TextPos)),
				CountStr, CountFont, ESlateDrawEffect::None, FLinearColor::White);
		}
	}

	Layer = TextLayer + 1;

	// --- 4. Tooltip / children on top ---
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, Layer, InWidgetStyle, bParentEnabled);
}

void STalentTreeWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (Nodes.IsValidIndex(HoveredNodeIndex))
	{
		HoverElapsed += InDeltaTime;
	}
	RefreshTooltip();
}

void STalentTreeWidget::RefreshTooltip()
{
	if (!TooltipContainer.IsValid())
	{
		return;
	}

	if (!Nodes.IsValidIndex(HoveredNodeIndex) || HoverElapsed < TooltipDelaySeconds)
	{
		TooltipContainer->SetVisibility(EVisibility::Collapsed);
		return;
	}

	const FTalentTreeSlateNode& Node = Nodes[HoveredNodeIndex];

	if (TooltipTitle.IsValid())
	{
		const FText Title = Node.ToolTipTitle.IsEmpty() ? Node.DisplayName : Node.ToolTipTitle;
		TooltipTitle->SetText(Title);
	}
	if (TooltipBody.IsValid())
	{
		TooltipBody->SetText(Node.ToolTipText);
		TooltipBody->SetVisibility(Node.ToolTipText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
	}
	if (TooltipPoints.IsValid())
	{
		const int32 Pts = GetNodePoints(Node.Id);
		TooltipPoints->SetText(FText::FromString(FString::Printf(TEXT("Points: %d / %d"), Pts, FMath::Max(1, Node.MaxPoints))));
	}
	if (TooltipIcon.IsValid())
	{
		if (Node.bHasIcon)
		{
			TooltipIconBrush = Node.IconBrush;
			TooltipIconBrush.ImageSize = FVector2D(28.f, 28.f);
			TooltipIcon->SetVisibility(EVisibility::HitTestInvisible);
		}
		else
		{
			TooltipIcon->SetVisibility(EVisibility::Collapsed);
		}
	}

	// Follow the cursor (render-transform only: visual, non-interactive).
	TooltipContainer->SetRenderTransform(FSlateRenderTransform(FVector2f((float)LocalMousePos.X + 18.f, (float)LocalMousePos.Y + 18.f)));
	TooltipContainer->SetVisibility(EVisibility::HitTestInvisible);
}

FReply STalentTreeWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		const FVector2D Local = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const int32 Idx = HitTestNode(Local, MyGeometry.GetLocalSize());
		if (Nodes.IsValidIndex(Idx))
		{
			OnInvestDelegate.ExecuteIfBound(Nodes[Idx].Id);
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

FReply STalentTreeWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	LocalMousePos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const int32 Idx = HitTestNode(LocalMousePos, MyGeometry.GetLocalSize());
	if (Idx != HoveredNodeIndex)
	{
		HoveredNodeIndex = Idx;
		HoverElapsed = 0.f;
		if (TooltipContainer.IsValid())
		{
			TooltipContainer->SetVisibility(EVisibility::Collapsed);
		}
	}
	return FReply::Unhandled();
}

void STalentTreeWidget::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	SCompoundWidget::OnMouseLeave(MouseEvent);
	HoveredNodeIndex = INDEX_NONE;
	HoverElapsed = 0.f;
	if (TooltipContainer.IsValid())
	{
		TooltipContainer->SetVisibility(EVisibility::Collapsed);
	}
}
