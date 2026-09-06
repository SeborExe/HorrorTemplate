// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemTypes.generated.h"

class UTexture2D;

/**
 *  Which hand an item is equipped in
 */
UENUM(BlueprintType)
enum class EItemEquipHand : uint8
{
	/** Item can be equipped in either hand */
	Any		UMETA(DisplayName = "Any Hand"),

	/** Item is equipped in the left hand */
	Left	UMETA(DisplayName = "Left Hand"),

	/** Item is equipped in the right hand */
	Right	UMETA(DisplayName = "Right Hand")
};

/**
 *  Holds all descriptive data for an item.
 *  Populated by Blueprint or C++ subclasses of AItemBase.
 */
USTRUCT(BlueprintType)
struct FItemData
{
	GENERATED_BODY()

	/** Localized display name shown in the UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	FText DisplayName;

	/** Icon shown in the UI. Soft reference so it's only loaded when needed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** If true, holding this item occupies both hands and EquipHand is ignored */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item")
	bool bRequiresBothHands = false;

	/** Which hand this item is equipped in when it doesn't require both */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item", meta = (EditCondition = "!bRequiresBothHands"))
	EItemEquipHand EquipHand = EItemEquipHand::Any;
};
