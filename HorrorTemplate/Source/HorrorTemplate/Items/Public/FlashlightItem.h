// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LightItem.h"
#include "FlashlightItem.generated.h"

/**
 *  Battery-powered flashlight. Overrides the inherited light component's class to a
 *  spot light so it casts a directional cone-shaped beam, and flickers the beam
 *  while the battery is low.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AFlashlightItem : public ALightItem
{
	GENERATED_BODY()

protected:

	/** Chance, each low-battery drain tick, that the beam flickers off and back on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Battery", meta = (ClampMin = 0, ClampMax = 1))
	float LowBatteryFlickerChance = 0.5f;

	/** How long the beam stays off during a low-battery flicker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Battery", meta = (ClampMin = 0, Units = "s"))
	float FlickerOffDuration = 0.1f;

	/** Timer used to switch the beam back on after a low-battery flicker */
	FTimerHandle FlickerTimer;

public:

	/** Constructor */
	AFlashlightItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Switches the light off */
	virtual void TurnOff() override;

	/** Randomly flickers the beam off and back on while the battery is low */
	virtual void LowBatteryResponse() override;

	/** Restores the beam after a flicker, if the flashlight is still switched on */
	void EndFlicker();
};
