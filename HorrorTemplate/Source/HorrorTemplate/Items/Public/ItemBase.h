// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemTypes.h"
#include "ItemBase.generated.h"

class UStaticMeshComponent;

/**
 *  Abstract base class for all pickupable / equippable items.
 *  Provides the item's descriptive data (FItemData) and a mesh root that
 *  can be highlighted through the Custom Depth buffer for an outline effect.
 *  Concrete behavior lives in C++ subclasses or in Blueprint subclasses.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AItemBase : public AActor
{
	GENERATED_BODY()

	/** Visible mesh and root component for the item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;

protected:

	/** Descriptive data for this item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FItemData ItemData;

	/** Stencil value written to the Custom Depth buffer while highlighted. Read by the outline post process material */
	UPROPERTY(EditAnywhere, Category="Item|Highlight", meta = (ClampMin = 0, ClampMax = 255))
	int32 HighlightStencilValue = 1;

	/** True while the item is currently highlighted */
	bool bIsHighlighted = false;

public:

	/** Constructor */
	AItemBase();

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

	/** Enables the Custom Depth outline for this item */
	UFUNCTION(BlueprintCallable, Category="Item")
	virtual void Highlight();

	/** Disables the Custom Depth outline for this item */
	UFUNCTION(BlueprintCallable, Category="Item")
	virtual void Unhighlight();

	/** Passes highlight state changes to Blueprint for extra feedback (sound, VFX) */
	UFUNCTION(BlueprintImplementableEvent, Category="Item", meta = (DisplayName = "OnHighlightChanged"))
	void BP_OnHighlightChanged(bool bHighlighted);

	/** Returns the item's descriptive data */
	UFUNCTION(BlueprintPure, Category="Item")
	const FItemData& GetItemData() const { return ItemData; }

	/** Returns the item's localized display name */
	UFUNCTION(BlueprintPure, Category="Item")
	FText GetDisplayName() const { return ItemData.DisplayName; }

	/** Returns whether this item occupies both hands */
	UFUNCTION(BlueprintPure, Category="Item")
	bool RequiresBothHands() const { return ItemData.bRequiresBothHands; }

	/** Returns which hand this item is equipped in */
	UFUNCTION(BlueprintPure, Category="Item")
	EItemEquipHand GetEquipHand() const { return ItemData.EquipHand; }

	/** Returns whether the item is currently highlighted */
	UFUNCTION(BlueprintPure, Category="Item")
	bool IsHighlighted() const { return bIsHighlighted; }

	/** Returns the mesh component */
	UFUNCTION(BlueprintPure, Category="Item")
	UStaticMeshComponent* GetMesh() const { return Mesh; }
};
