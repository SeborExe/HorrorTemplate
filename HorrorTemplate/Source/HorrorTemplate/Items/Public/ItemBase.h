// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ItemTypes.h"
#include "ItemBase.generated.h"

class UStaticMeshComponent;

/**
 *  Abstract base class for every item.
 *  Provides the descriptive data (FItemData), an identifying gameplay tag and a
 *  mesh root that can be highlighted through the Custom Depth buffer for an
 *  outline effect.
 *  Subclasses declare what the player can do with the item by overriding
 *  CanBeInspected / CanBePickedUp / CanBeEquipped:
 *   - AItemEquippable  inspect + pick up + equip   (e.g. flashlight)
 *   - AItemUsable      inspect + pick up           (e.g. key)
 *   - AItemInspectable inspect only                (view-only prop)
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

	/** Identifies this item type. Used for lookups, lock / quest matching, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	FGameplayTag ItemTag;

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

	/** Whether the player can hold this item up in front of the camera and inspect it */
	UFUNCTION(BlueprintPure, Category="Item")
	virtual bool CanBeInspected() const { return true; }

	/** Whether the player can pick this item up into the inventory */
	UFUNCTION(BlueprintPure, Category="Item")
	virtual bool CanBePickedUp() const { return false; }

	/** Whether this item can be equipped into a hand slot */
	UFUNCTION(BlueprintPure, Category="Item")
	virtual bool CanBeEquipped() const { return false; }

	/** Returns the item's descriptive data */
	UFUNCTION(BlueprintPure, Category="Item")
	const FItemData& GetItemData() const { return ItemData; }

	/** Returns the item's identifying gameplay tag */
	UFUNCTION(BlueprintPure, Category="Item")
	FGameplayTag GetItemTag() const { return ItemTag; }

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
