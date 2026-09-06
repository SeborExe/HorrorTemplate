// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorMainPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "HorrorTemplate.h"

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
