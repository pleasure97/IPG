// Fill out your copyright notice in the Description page of Project Settings.


#include "Fragment/ItemFragment.h"
#include "Widget/Composite/CompositeBaseWidget.h"
#include "Widget/Composite/LeafImageWidget.h"
#include "Widget/Composite/LeafTextWidget.h"
#include "Widget/Composite/LeafValueWidget.h"
#include "Equipment/EquipmentActor.h"

/*----------------------------------------------------- Inventory Item ----------------------------------------------------- */
// Assimilate leaf widgets
void FInventoryItemFragment::Assimilate(UCompositeBaseWidget* Composite) const
{
	if (!MatchesWidgetTag(Composite))
	{
		return;
	}
	Composite->Expand();
}

bool FInventoryItemFragment::MatchesWidgetTag(const UCompositeBaseWidget* Composite) const
{
	return Composite->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

/*----------------------------------------------------- Grid ----------------------------------------------------- */
/* Grid Size */
FIntPoint FGridFragment::GetGridSize() const
{
	return GridSize;
}
void FGridFragment::SetGridSize(const FIntPoint& Size)
{
	GridSize = Size;
}

/* Grid Padding */
float FGridFragment::GetGridPadding() const
{
	return GridPadding;
}

void FGridFragment::SetGridPadding(float Padding)
{
	GridPadding = Padding;
}

/*----------------------------------------------------- Consumable ----------------------------------------------------- */
void FConsumableFragment::OnConsume(APlayerController* PC)
{
	for (TInstancedStruct<FConsumeModifier>& ConsumeModifier : ConsumeModifiers)
	{
		FConsumeModifier& ConsumeModifierRef = ConsumeModifier.GetMutable();
		ConsumeModifierRef.OnConsume(PC);
	}
}

void FConsumableFragment::Manifest()
{
	FItemFragment::Manifest();
	for (TInstancedStruct<FConsumeModifier>& ConsumeModifier : ConsumeModifiers)
	{
		FConsumeModifier& ConsumeModifierRef = ConsumeModifier.GetMutable();
		ConsumeModifierRef.Manifest();
	}
}

/*----------------------------------------------------- Image ----------------------------------------------------- */
UTexture2D* FImageFragment::GetIcon() const
{
	return Icon;
}

void FImageFragment::Assimilate(UCompositeBaseWidget* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite); 

	if (!MatchesWidgetTag(Composite))
	{
		return;
	}

	ULeafImageWidget* LeafImageWidget = Cast<ULeafImageWidget>(Composite); 
	if (!IsValid(LeafImageWidget))
	{
		return;
	}

	LeafImageWidget->SetImage(Icon); 
	LeafImageWidget->SetBoxSize(IconDimensions);
	LeafImageWidget->SetImageSize(IconDimensions);
}

/*----------------------------------------------------- Text ----------------------------------------------------- */
FText FTextFragment::GetText() const
{
	return FragmentText;
}

void FTextFragment::SetText(const FText& Text)
{
	FragmentText = Text;
}

void FTextFragment::Assimilate(UCompositeBaseWidget* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);

	if (!MatchesWidgetTag(Composite))
	{
		return;
	}

	ULeafTextWidget* LeafTextWidget = Cast<ULeafTextWidget>(Composite); 
	if (!IsValid(LeafTextWidget))
	{
		return;
	}

	LeafTextWidget->SetText(FragmentText);
}

/*----------------------------------------------------- Value ----------------------------------------------------- */
void FValueFragment::Assimilate(UCompositeBaseWidget* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite); 

	if (!MatchesWidgetTag(Composite))
	{
		return;
	}

	ULeafValueWidget* LeafValueWidget = Cast<ULeafValueWidget>(Composite); 
	if (!IsValid(LeafValueWidget))
	{
		return;
	}

	LeafValueWidget->SetLabelText(LabelText, bCollapseLabel); 

	FNumberFormattingOptions NumberFormattingOptions;
	NumberFormattingOptions.MinimumFractionalDigits = MinFractionalDigits;
	NumberFormattingOptions.MaximumFractionalDigits = MaxFractionalDigits;

	LeafValueWidget->SetValueText(FText::AsNumber(Value, &NumberFormattingOptions), bCollapseValue);
}

void FValueFragment::Manifest()
{
	FInventoryItemFragment::Manifest(); 

	if (bRandomizeOnManifest)
	{
		Value = FMath::RandRange(Min, Max); 
	}
	bRandomizeOnManifest = false;
}

float FValueFragment::GetValue() const
{
	return Value;
}

void FValueFragment::SetRandomizeOnManifest(bool bInRandomizeOnManifest)
{
	bRandomizeOnManifest = bInRandomizeOnManifest;
}

/*----------------------------------------------------- Equipment ----------------------------------------------------- */
FGameplayTag FEquipmentFragment::GetEquipmentType() const
{
	return EquipmentTypeTag;
}

TSubclassOf<AEquipmentActor> FEquipmentFragment::GetEquipActorClass() const
{
	return EquipActorClass;
}

FName FEquipmentFragment::GetSocketAttachPoint() const
{
	return SocketAttachPoint;
}

void FEquipmentFragment::SetEquipActor(AEquipmentActor* InEquipActor)
{
	EquipActor = InEquipActor;
}

AEquipmentActor* FEquipmentFragment::SpawnEquipActor(USkeletalMeshComponent* EquipMesh) const
{
	if (!IsValid(EquipActorClass) || !IsValid(EquipMesh))
	{
		return nullptr;
	}

	AEquipmentActor* SpawnedEquipmentActor = EquipMesh->GetWorld()->SpawnActor<AEquipmentActor>(EquipActorClass);
	SpawnedEquipmentActor->AttachToComponent(EquipMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint);

	return SpawnedEquipmentActor;
}

void FEquipmentFragment::DestroyEquipActor() const
{
	if (EquipActor.IsValid())
	{
		EquipActor->Destroy();
	}
}

void FEquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped)
	{
		return;
	}
	bEquipped = true;
	for (auto& EquipModifier : EquipModifiers)
	{
		auto& EquipModifierRef = EquipModifier.GetMutable();
		EquipModifierRef.OnEquip(PC);
	}
}

void FEquipmentFragment::OnUnequip(APlayerController* PC)
{
	if (!bEquipped)
	{
		return;
	}
	bEquipped = false;
	for (auto& EquipModifier : EquipModifiers)
	{
		auto& EquipModifierRef = EquipModifier.GetMutable(); 
		EquipModifierRef.OnUnequip(PC);
	}
}

void FEquipmentFragment::Assimilate(UCompositeBaseWidget* Composite) const
{
	FInventoryItemFragment::Assimilate(Composite);
	for (const auto& EquipModifier : EquipModifiers)
	{
		const auto& EquipModifierRef = EquipModifier.Get(); 
		EquipModifierRef.Assimilate(Composite);
	}
}

void FEquipmentFragment::Manifest()
{
	FInventoryItemFragment::Manifest();
	for (auto& EquipModifier : EquipModifiers)
	{
		auto& EquipModifierRef = EquipModifier.GetMutable();
		EquipModifierRef.Manifest();
	}
}
