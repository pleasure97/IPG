// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "ItemFragment.generated.h"

class UCompositeBaseWidget;
class AEquipmentActor;
/**
 * 
 */
USTRUCT()
struct FItemFragment
{
	GENERATED_BODY()

public:
	FItemFragment() {}
	FItemFragment(const FItemFragment&) = default;
	FItemFragment& operator=(const FItemFragment&) = default;
	FItemFragment(FItemFragment&&) = default;
	FItemFragment& operator=(FItemFragment&&) = default;
	virtual ~FItemFragment() {}

	FGameplayTag GetFragmentTag() const { return FragmentTag; }
	void SetFragmentTag(FGameplayTag Tag) { FragmentTag = Tag; }

	virtual void Manifest() {}

private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories = "FragmentTags"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

/* Item fragment specifically for assimilation into a widget */
USTRUCT(BlueprintType)
struct FInventoryItemFragment : public FItemFragment
{
	GENERATED_BODY()

public:
	virtual void Assimilate(UCompositeBaseWidget* Composite) const;

protected:
	bool MatchesWidgetTag(const UCompositeBaseWidget* Composite) const;
};

/* Grid */
USTRUCT(BlueprintType)
struct FGridFragment : public FItemFragment
{
	GENERATED_BODY()

public:
	/* Grid Size */
	FIntPoint GetGridSize() const; 
	void SetGridSize(const FIntPoint& Size); 

	/* Grid Padding */
	float GetGridPadding() const; 
	void SetGridPadding(float Padding); 

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FIntPoint GridSize = FIntPoint(1, 1); 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float GridPadding = 0.f;
};

/* Image */
USTRUCT(BlueprintType)
struct FImageFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

public:
	UTexture2D* GetIcon() const; 
	virtual void Assimilate(UCompositeBaseWidget* Composite) const override;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D IconDimensions = FVector2D(44.f, 44.f);
};

/* Text */
USTRUCT(BlueprintType)
struct FTextFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

public:
	FText GetText() const;
	void SetText(const FText& Text); 

	virtual void Assimilate(UCompositeBaseWidget* Composite) const override;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText FragmentText;
};

/* Value */
USTRUCT(BlueprintType)
struct FValueFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void Assimilate(UCompositeBaseWidget* Composite) const override;
	virtual void Manifest() override; 

	float GetValue() const; 
	void SetRandomizeOnManifest(bool bInRandomizeOnManifest);

private:
	bool bRandomizeOnManifest = true; 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText LabelText; 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Value = 0.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Min = 0.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Max = 0.f;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseLabel = false;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseValue = false;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MinFractionalDigits = 1; 

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxFractionalDigits = 1; 
};

/* Stackable */
USTRUCT(BlueprintType)
struct FStackableFragment : public FItemFragment
{
	GENERATED_BODY()

public:
	int32 GetMaxStackSize() const { return MaxStackSize; }
	int32 GetStackCount() const { return StackCount; }
	void SetStackCount(int32 Count) { StackCount = Count; }

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 StackCount = 1;
};

/* Equipment */
USTRUCT(BlueprintType)
struct FEquipModifier : public FValueFragment
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) {}
	virtual void OnUnequip(APlayerController* PC) {}
};


USTRUCT(BlueprintType)
struct FEquipmentFragment : public FInventoryItemFragment
{
	GENERATED_BODY()

public:
	FGameplayTag GetEquipmentType() const; 
	void SetEquipActor(AEquipmentActor* InEquipActor);

	AEquipmentActor* SpawnEquipActor(USkeletalMeshComponent* EquipMesh) const;
	void DestroyEquipActor() const;

protected:
	virtual void Assimilate(UCompositeBaseWidget* Composite) const override;
	virtual void Manifest() override; 
	
	void OnEquip(APlayerController* PC); 
	void OnUnequip(APlayerController* PC);

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<TInstancedStruct<FEquipModifier>> EquipModifiers;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AEquipmentActor> EquipActorClass = nullptr;

	TWeakObjectPtr<AEquipmentActor> EquipActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FName SocketAttachPoint = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentTypeTag = FGameplayTag::EmptyTag;

	bool bEquipped = false;
};

/* Consumable */
USTRUCT(BlueprintType)
struct FConsumeModifier : public FItemFragment
{
	GENERATED_BODY()

public:
	virtual void OnConsume(APlayerController* PC) {}
};

USTRUCT(BlueprintType)
struct FConsumableFragment : public FItemFragment
{
	GENERATED_BODY()

public:
	virtual void OnConsume(APlayerController* PC); 
	virtual void Manifest() override; 

private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FConsumeModifier>> ConsumeModifiers;
};