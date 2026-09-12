// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ItemEquippable.generated.h"

/**
 *  Item that can be equipped into a hand slot, e.g. a flashlight.
 *  Can also be inspected and picked up.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AItemEquippable : public AItemBase
{
	GENERATED_BODY()

protected:

	/**
	 *  Skeletal socket this item attaches to when equipped. Author the socket on the
	 *  character's first person mesh to control exactly how the item sits in the hand.
	 *  Leave as None to fall back to the inventory's default hand socket.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Equip")
	FName SocketName;

	/** Relative scale applied to the item while it's equipped in a hand */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Equip")
	FVector HandScale = FVector::OneVector;

public:

	virtual bool CanBeInspected() const override { return true; }
	virtual bool CanBePickedUp() const override { return true; }
	virtual bool CanBeEquipped() const override { return true; }

	/** Returns the equip socket, or NAME_None to use the default hand socket */
	UFUNCTION(BlueprintPure, Category="Item|Equip")
	FName GetSocketName() const { return SocketName; }

	/** Returns the relative scale to use while equipped */
	UFUNCTION(BlueprintPure, Category="Item|Equip")
	FVector GetHandScale() const { return HandScale; }

	/**
	 *  Activates the item's primary use, e.g. toggling a flashlight on/off.
	 *  Bound to the mouse button matching the hand this item is equipped in.
	 *  Empty by default; subclasses override it for their own behavior.
	 */
	UFUNCTION(BlueprintCallable, Category="Item|Equip")
	virtual void UseItem();
};
