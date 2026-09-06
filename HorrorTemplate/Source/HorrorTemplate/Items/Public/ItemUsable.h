// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ItemUsable.generated.h"

/**
 *  Item that can be inspected and picked up into the inventory but not equipped
 *  into a hand, e.g. a key.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AItemUsable : public AItemBase
{
	GENERATED_BODY()

public:

	virtual bool CanBeInspected() const override { return true; }
	virtual bool CanBePickedUp() const override { return true; }
	virtual bool CanBeEquipped() const override { return false; }
};
