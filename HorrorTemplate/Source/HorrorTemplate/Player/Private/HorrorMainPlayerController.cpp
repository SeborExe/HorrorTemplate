// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorMainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InventoryComponent.h"
#include "HorrorTemplate.h"

AHorrorMainPlayerController::AHorrorMainPlayerController()
{
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void AHorrorMainPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// mapping contexts are per local player
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (const TObjectPtr<UInputMappingContext>& MappingContext : DefaultMappingContexts)
		{
			if (MappingContext)
			{
				Subsystem->AddMappingContext(MappingContext, InputMappingPriority);
			}
		}
	}
	else
	{
		UE_LOG(LogHorrorTemplate, Error, TEXT("'%s' could not find the EnhancedInputLocalPlayerSubsystem. This project requires Enhanced Input."), *GetNameSafe(this));
	}
}

void AHorrorMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// GetPawn() is valid past Super::OnPossess, so the inventory can attach to it
	if (Inventory)
	{
		Inventory->HandlePawnChanged(InPawn);
	}
}

void AHorrorMainPlayerController::OnUnPossess()
{
	if (Inventory)
	{
		Inventory->HandlePawnChanged(nullptr);
	}

	Super::OnUnPossess();
}
