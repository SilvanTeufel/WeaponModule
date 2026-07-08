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
	Super::ReleaseSlateResources(bReleaseChildren);
	MyTalentTree.Reset();
}

void UTalentTreeWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	RefreshNodes();
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
