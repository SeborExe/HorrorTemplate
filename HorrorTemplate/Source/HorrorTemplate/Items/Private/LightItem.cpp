// Copyright Epic Games, Inc. All Rights Reserved.


#include "LightItem.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

FName ALightItem::LightComponentName(TEXT("LightComponent"));

ALightItem::ALightItem(const FObjectInitializer& ObjectInitializer)
{
	LightComponent = CreateDefaultSubobject<UPointLightComponent>(LightComponentName);
	LightComponent->SetupAttachment(GetMesh());
	LightComponent->SetVisibility(false);
}

void ALightItem::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the battery drain timer
	GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimer);
}

void ALightItem::UseItem()
{
	IsOn() ? TurnOff() : TurnOn();
}

void ALightItem::TurnOn()
{
	if (bIsOn)
	{
		return;
	}

	bIsOn = true;
	LightComponent->SetVisibility(true);

	// only drain the battery if it isn't infinite and there's charge left to drain
	if (!bIsInfinite && BatteryLevel > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(BatteryDrainTimer, this, &ALightItem::BatteryDrainTick, BatteryDrainTickInterval, true);
	}
}

void ALightItem::TurnOff()
{
	if (!bIsOn)
	{
		return;
	}

	bIsOn = false;
	LightComponent->SetVisibility(false);

	GetWorld()->GetTimerManager().ClearTimer(BatteryDrainTimer);
}

void ALightItem::BatteryDrainTick()
{
	const float DrainThisTick = (100.0f / BatteryLifeSeconds) * BatteryDrainTickInterval;
	BatteryLevel = FMath::Max(BatteryLevel - DrainThisTick, 0.0f);

	if (IsBatteryLow())
	{
		LowBatteryResponse();
	}

	// out of charge - switch off
	if (BatteryLevel <= 0.0f)
	{
		TurnOff();
	}
}

void ALightItem::LowBatteryResponse()
{
	// implemented by subclasses, e.g. AFlashlightItem, to flicker the light
}
