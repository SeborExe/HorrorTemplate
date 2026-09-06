// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase.h"
#include "ItemInspectable.generated.h"

/**
 *  View-only item. The player can hold it up and look at it but cannot pick it
 *  up or equip it.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AItemInspectable : public AItemBase
{
	GENERATED_BODY()

public:

	virtual bool CanBeInspected() const override { return true; }
	virtual bool CanBePickedUp() const override { return false; }
	virtual bool CanBeEquipped() const override { return false; }
};
