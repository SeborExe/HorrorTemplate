// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemEquippable.h"
#include "LightItem.generated.h"

class UPointLightComponent;

/**
 *  Base class for every carryable light source, e.g. a flashlight or an oil lamp.
 *  The light source is a UPointLightComponent - USpotLightComponent derives from it,
 *  so a directional subclass (a flashlight) can swap it in via
 *  FObjectInitializer::SetDefaultSubobjectClass in its own constructor, while an
 *  omnidirectional subclass (an oil lamp) can use the point light as-is.
 *  Drains a percentage-based battery on a fixed-interval timer while switched on,
 *  unless bIsInfinite is set. LowBatteryResponse is called every drain tick once the
 *  charge is at or below LowBatteryThreshold; subclasses override it to flicker.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API ALightItem : public AItemEquippable
{
	GENERATED_BODY()

	/** Light source. A point light so subclasses can swap in a spot light (e.g. a flashlight) via the object initializer */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> LightComponent;

protected:

	/** If true, the light never drains its battery */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Battery")
	bool bIsInfinite = false;

	/** How many seconds of continuous use a full battery lasts, when not infinite */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Battery", meta = (EditCondition = "!bIsInfinite", ClampMin = 1, Units = "s"))
	float BatteryLifeSeconds = 300.0f;

	/** Battery charge percentage, from 0 (dead) to 100 (full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Battery", meta = (EditCondition = "!bIsInfinite", ClampMin = 0, ClampMax = 100))
	float BatteryLevel = 100.0f;

	/** Battery percentage at or below which the light is considered low. LowBatteryResponse is called every drain tick while this low */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item|Battery", meta = (EditCondition = "!bIsInfinite", ClampMin = 0, ClampMax = 100))
	float LowBatteryThreshold = 20.0f;

	/** Time interval for battery drain ticks */
	UPROPERTY(EditAnywhere, Category="Item|Battery", meta = (EditCondition = "!bIsInfinite", ClampMin = 0.05, ClampMax = 5, Units = "s"))
	float BatteryDrainTickInterval = 0.5f;

	/** True while the light is switched on */
	bool bIsOn = false;

	/** Battery drain tick timer */
	FTimerHandle BatteryDrainTimer;

public:

	/** Name of the light component. Override its class in a subclass constructor via ObjectInitializer.SetDefaultSubobjectClass, e.g. to swap in a spot light for a flashlight */
	static FName LightComponentName;

	/** Constructor */
	ALightItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Called on a fixed interval while the light is on to drain the battery */
	void BatteryDrainTick();

	/** Called every drain tick once the battery is at or below LowBatteryThreshold. Empty here - e.g. AFlashlightItem overrides it to flicker the light */
	virtual void LowBatteryResponse();

public:

	/** Toggles the light on or off depending on its current state */
	virtual void UseItem() override;

	/** Switches the light on */
	UFUNCTION(BlueprintCallable, Category="Item|Light")
	virtual void TurnOn();

	/** Switches the light off */
	UFUNCTION(BlueprintCallable, Category="Item|Light")
	virtual void TurnOff();

	/** Returns whether the light is currently switched on */
	UFUNCTION(BlueprintPure, Category="Item|Light")
	bool IsOn() const { return bIsOn; }

	/** Returns whether this light has an infinite battery */
	UFUNCTION(BlueprintPure, Category="Item|Battery")
	bool IsInfinite() const { return bIsInfinite; }

	/** Returns the current battery charge percentage (0-100) */
	UFUNCTION(BlueprintPure, Category="Item|Battery")
	float GetBatteryLevel() const { return BatteryLevel; }

	/** Returns whether the battery is at or below the low battery threshold */
	UFUNCTION(BlueprintPure, Category="Item|Battery")
	bool IsBatteryLow() const { return !bIsInfinite && BatteryLevel <= LowBatteryThreshold; }

	/** Returns the light component */
	UFUNCTION(BlueprintPure, Category="Item|Light")
	UPointLightComponent* GetLightComponent() const { return LightComponent; }
};
