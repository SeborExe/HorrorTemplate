// Copyright Epic Games, Inc. All Rights Reserved.


#include "FlashlightItem.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

AFlashlightItem::AFlashlightItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USpotLightComponent>(LightComponentName))
{
}

void AFlashlightItem::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(FlickerTimer);
}

void AFlashlightItem::TurnOff()
{
	Super::TurnOff();

	GetWorld()->GetTimerManager().ClearTimer(FlickerTimer);
}

void AFlashlightItem::LowBatteryResponse()
{
	if (!IsOn())
	{
		return;
	}

	if (FMath::FRand() <= LowBatteryFlickerChance)
	{
		GetLightComponent()->SetVisibility(false);
		GetWorld()->GetTimerManager().SetTimer(FlickerTimer, this, &AFlashlightItem::EndFlicker, FlickerOffDuration, false);
	}
}

void AFlashlightItem::EndFlicker()
{
	// the flashlight may have been switched off while the beam was mid-flicker
	if (IsOn())
	{
		GetLightComponent()->SetVisibility(true);
	}
}
