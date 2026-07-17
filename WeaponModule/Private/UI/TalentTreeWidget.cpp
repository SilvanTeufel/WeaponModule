// Copyright 2026 Silvan Teufel / Teufel-Engineering.com All Rights Reserved.

#include "UI/TalentTreeWidget.h"
#include "UI/STalentTreeWidget.h"
#include "Components/WeaponComponent.h"
#include "Components/WeaponHUDComponent.h"
#include "Characters/Unit/UnitBase.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Brushes/SlateColorBrush.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

// A Slate brush material MUST have Material Domain = "User Interface". This is enforced only at shader
// permutation level (the Slate material shaders compile solely for MD_UI) and FSlateBrush's own ensure
// checks the CLASS only - so a Surface-domain material sails through, finds no shader, and draws
// NOTHING: no crash, no checkerboard, no log, indistinguishable from "no material set". That is
// undiagnosable in the Details panel, hence this guard.
// IsUIMaterial() is virtual on UMaterialInterface and UMaterialInstance forwards it to Parent, so this
// is correct for UMaterial, material instances and MIDs alike.
static void WarnIfNotUIMaterial(const FSlateBrush& Brush, const TCHAR* PropName, const UObject* Owner)
{
	if (UMaterialInterface* Mat = Cast<UMaterialInterface>(Brush.GetResourceObject()))
	{
		if (!Mat->IsUIMaterial())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s (%s): Material '%s' has Material Domain != 'User Interface'. ")
				TEXT("Slate compiles no shader for it and will draw NOTHING (silently). Set Material Domain = User Interface."),
				*GetNameSafe(Owner), PropName, *Mat->GetName());
		}
	}
}

UTalentTreeWidget::UTalentTreeWidget()
{
	// Byte-identical to the brush the SWidget used to hard-code: DrawAs=Image, ImageType=NoImage,
	// Margin=0, TintColor=White. White * BackgroundColor == BackgroundColor, so existing content
	// renders exactly as before.
	BackgroundBrush = FSlateColorBrush(FLinearColor::White);
	// Non-zero base size so a later flip to Draw As = Box 9-slices sensibly. Inert for the default: the
	// 9-quad path is gated on DrawType != Image && Margin != 0, and tiling is NoTile.
	BackgroundBrush.ImageSize = FVector2f(64.f, 64.f);

	// No border until opted in -> preserves today's look exactly. Must be set explicitly: FSlateBrush's
	// default ctor is DrawAs=Image / ImageSize 32x32, which would paint an opaque white box over every
	// existing tree on first run.
	BorderBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
	BorderBrush.TintColor = FSlateColor(FLinearColor::White);
	// Ready-to-use 9-slice: for a MATERIAL, ImageSize is the ONLY size source (no texture to measure)
	// and corners = ImageSize * Margin. At the engine's 32x32 default a large frame would get 8px
	// corners and read as "the border is broken"; at ImageSize 0 they collapse entirely.
	BorderBrush.ImageSize = FVector2f(256.f, 256.f);
	BorderBrush.Margin = FMargin(0.25f);
}

TSharedRef<SWidget> UTalentTreeWidget::RebuildWidget()
{
	MyTalentTree = SNew(STalentTreeWidget)
		.RingSpacing(RingSpacing)
		.NodeRadius(NodeRadius)
		.TooltipDelaySeconds(TooltipDelaySeconds)
		.LockedColor(LockedColor)
		.AvailableColor(AvailableColor)
		.PartialColor(PartialColor)
		.FullColor(FullColor)
		.LinkColor(LinkColor)
		.RingColor(RingColor)
		.BackgroundColor(BackgroundColor)
		.bFillViewport(bFillViewport)
		// Address of this UObject's own UPROPERTY - the UBorder pattern verbatim. Never a temporary:
		// the UPROPERTY is the GC anchor for whatever material/texture the brush holds.
		.BackgroundBrush(&BackgroundBrush)
		.BorderBrush(&BorderBrush)
		.BorderPadding(BorderPadding)
		.NodeFontSize(NodeFontSize)
		.CountFontSize(CountFontSize)
		.TooltipTitleFontSize(TooltipTitleFontSize)
		.TooltipBodyFontSize(TooltipBodyFontSize)
		.TooltipMaxWidth(TooltipMaxWidth)
		.TooltipPadding(TooltipPadding)
		.TooltipIconSize(TooltipIconSize)
		.TooltipIconGap(TooltipIconGap)
		.OnGetNodePoints(FTalentTreeGetNodePoints::CreateUObject(this, &UTalentTreeWidget::HandleGetNodePoints))
		.OnGetAvailablePoints(FTalentTreeGetAvailablePoints::CreateUObject(this, &UTalentTreeWidget::HandleGetAvailablePoints))
		.OnIsUnlocked(FTalentTreeIsUnlocked::CreateUObject(this, &UTalentTreeWidget::HandleIsUnlocked))
		.OnInvest(FTalentTreeOnInvest::CreateUObject(this, &UTalentTreeWidget::HandleInvest))
		.OnReset(FTalentTreeOnReset::CreateUObject(this, &UTalentTreeWidget::HandleReset));

	RefreshNodes();
	return MyTalentTree.ToSharedRef();
}

void UTalentTreeWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	// Null the borrowed brush pointers BEFORE dropping our reference. Reached from UVisual::BeginDestroy,
	// i.e. while this UObject's brush memory is still fully valid. If anything else still holds a shared
	// ref to the SWidget it now paints its own fallback instead of reading freed UObject memory.
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(nullptr, nullptr, BackgroundColor, BorderPadding);
	}
	Super::ReleaseSlateResources(bReleaseChildren);
	MyTalentTree.Reset();
}

void UTalentTreeWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	// Re-push panel style so Details-panel edits live-update: UWidget::PostEditChangeProperty calls
	// SynchronizeProperties and never RebuildWidget, so construction-only SLATE_ARGUMENTs never refresh.
	// SCOPE, DELIBERATE: only the background/border style is pushed. The other style values (RingSpacing,
	// node colours, fonts, tooltip metrics) stay construction-only, so the Details panel IS inconsistent
	// by design. Extending it means one SWidget setter each, plus Invalidate(Layout) rather than Paint
	// for RingSpacing/NodeRadius - they feed ComputeDesiredSize and the brushes/fonts cached in Construct.
	if (MyTalentTree.IsValid())
	{
		WarnIfNotUIMaterial(BackgroundBrush, TEXT("BackgroundBrush"), this);
		WarnIfNotUIMaterial(BorderBrush, TEXT("BorderBrush"), this);
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
	RefreshNodes();
}

#if WITH_EDITOR
void UTalentTreeWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// Super drives SynchronizeProperties -> live update + the Material-Domain warning.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UTalentTreeWidget::SetBackgroundBrush(const FSlateBrush& InBrush)
{
	BackgroundBrush = InBrush;
	WarnIfNotUIMaterial(BackgroundBrush, TEXT("BackgroundBrush"), this);
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
}

void UTalentTreeWidget::SetBackgroundBrushFromMaterial(UMaterialInterface* Material)
{
	BackgroundBrush.SetResourceObject(Material);
	WarnIfNotUIMaterial(BackgroundBrush, TEXT("BackgroundBrush"), this);
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
}

void UTalentTreeWidget::SetBorderBrush(const FSlateBrush& InBrush)
{
	BorderBrush = InBrush;
	WarnIfNotUIMaterial(BorderBrush, TEXT("BorderBrush"), this);
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
}

void UTalentTreeWidget::SetBorderBrushFromMaterial(UMaterialInterface* Material)
{
	BorderBrush.SetResourceObject(Material);
	WarnIfNotUIMaterial(BorderBrush, TEXT("BorderBrush"), this);
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
}

void UTalentTreeWidget::SetBackgroundColor(FLinearColor InColor)
{
	BackgroundColor = InColor;
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
}

void UTalentTreeWidget::SetBorderPadding(FMargin InPadding)
{
	BorderPadding = InPadding;
	if (MyTalentTree.IsValid())
	{
		MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
	}
}

UMaterialInstanceDynamic* UTalentTreeWidget::GetBackgroundDynamicMaterial()
{
	UMaterialInterface* Material = Cast<UMaterialInterface>(BackgroundBrush.GetResourceObject());
	if (!Material)
	{
		return nullptr;
	}
	// Cast-to-MID first and return it if we already wrapped: without this guard every call re-wraps and
	// leaks a MID chain. (UBorder::GetDynamicMaterial does exactly this.)
	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);
	if (!DynamicMaterial)
	{
		// `this` (the UWidget) is the Outer - the Outer IS the keep-alive for the MID.
		DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
		BackgroundBrush.SetResourceObject(DynamicMaterial);
		if (MyTalentTree.IsValid())
		{
			MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
		}
	}
	return DynamicMaterial;
}

UMaterialInstanceDynamic* UTalentTreeWidget::GetBorderDynamicMaterial()
{
	UMaterialInterface* Material = Cast<UMaterialInterface>(BorderBrush.GetResourceObject());
	if (!Material)
	{
		return nullptr;
	}
	UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);
	if (!DynamicMaterial)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
		BorderBrush.SetResourceObject(DynamicMaterial);
		if (MyTalentTree.IsValid())
		{
			MyTalentTree->SetPanelStyle(&BackgroundBrush, &BorderBrush, BackgroundColor, BorderPadding);
		}
	}
	return DynamicMaterial;
}

void UTalentTreeWidget::SetTargetUnit(AUnitBase* InUnit)
{
	TargetUnit = InUnit;
	RefreshNodes();
}

UWeaponComponent* UTalentTreeWidget::GetWeaponComponent() const
{
	return TargetUnit ? TargetUnit->FindComponentByClass<UWeaponComponent>() : nullptr;
}

void UTalentTreeWidget::RefreshNodes()
{
	if (!MyTalentTree.IsValid())
	{
		return;
	}

	TArray<FTalentTreeSlateNode> SlateNodes;

	UWeaponComponent* WC = GetWeaponComponent();
	if (WC && WC->TalentTreeDataTable)
	{
		const TArray<FName> RowNames = WC->TalentTreeDataTable->GetRowNames();
		SlateNodes.Reserve(RowNames.Num());
		for (const FName& RowName : RowNames)
		{
			const FTalentTreeNodeRow* Row = WC->TalentTreeDataTable->FindRow<FTalentTreeNodeRow>(RowName, TEXT("TalentTreeUI"), /*bWarnIfMissing=*/false);
			if (!Row)
			{
				continue;
			}

			FTalentTreeSlateNode N;
			N.Id           = RowName;
			N.PrevId       = Row->PrevId;
			N.Ring         = Row->Ring;
			N.Angle        = Row->Angle;
			N.MaxPoints    = FMath::Max(1, Row->MaxPoints);
			N.DisplayName  = Row->DisplayName;
			N.ToolTipTitle = Row->ToolTipTitle;
			N.ToolTipText  = Row->ToolTipText;
			N.BaseColor    = Row->NodeColor;

			if (Row->Icon)
			{
				N.IconBrush.SetResourceObject(Row->Icon);
				N.IconBrush.ImageSize = FVector2D(NodeRadius * 1.6f, NodeRadius * 1.6f);
				N.IconBrush.DrawAs = ESlateBrushDrawType::Image;
				N.bHasIcon = true;
			}

			SlateNodes.Add(MoveTemp(N));
		}
	}

	MyTalentTree->SetNodes(MoveTemp(SlateNodes));
}

int32 UTalentTreeWidget::HandleGetNodePoints(FName NodeId) const
{
	UWeaponComponent* WC = GetWeaponComponent();
	return WC ? WC->GetTalentTreeNodePoints(NodeId) : 0;
}

int32 UTalentTreeWidget::HandleGetAvailablePoints() const
{
	UWeaponComponent* WC = GetWeaponComponent();
	return WC ? FMath::FloorToInt(WC->GetAvailableTalentTreePoints()) : 0;
}

bool UTalentTreeWidget::HandleIsUnlocked(FName NodeId) const
{
	UWeaponComponent* WC = GetWeaponComponent();
	return WC ? WC->IsTalentTreeNodeUnlocked(NodeId) : false;
}

void UTalentTreeWidget::HandleInvest(FName NodeId)
{
	UWeaponComponent* WC = GetWeaponComponent();
	if (!WC)
	{
		return;
	}

	// Client-side gate to avoid pointless RPCs; the server re-validates authoritatively.
	if (!WC->CanInvestInTalentTreeNode(NodeId))
	{
		return;
	}

	// Route through the player-controller-owned HUD component so the Server RPC has a valid owner
	// (mirrors the other talent widgets). Fall back to the component RPC in standalone.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			HUDComp->Server_InvestInTalentTreeNode(WC, NodeId);
			return;
		}
	}

	WC->Server_InvestInTalentTreeNode(NodeId);
}

void UTalentTreeWidget::HandleReset()
{
	UWeaponComponent* WC = GetWeaponComponent();
	if (!WC)
	{
		return;
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (UWeaponHUDComponent* HUDComp = PC->FindComponentByClass<UWeaponHUDComponent>())
		{
			HUDComp->Server_ResetTalentTree(WC);
			return;
		}
	}

	WC->Server_ResetTalentTree();
}
