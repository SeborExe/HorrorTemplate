// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorPlayerPawn.h"
#include "InteractionComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HorrorTemplate.h"

AHorrorPlayerPawn::AHorrorPlayerPawn()
{
	// movement is driven by input events and the character movement component's
	// own tick, so the pawn actor itself doesn't need to tick
	PrimaryActorTick.bCanEverTick = false;

	Interaction = CreateDefaultSubobject<UInteractionComponent>(TEXT("Interaction"));
}

void AHorrorPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	// start out walking
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WalkSpeed;
	}
}

void AHorrorPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// AHorrorTemplateCharacter binds move / look / jump through Enhanced Input.
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// hold to sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHorrorPlayerPawn::DoStartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHorrorPlayerPawn::DoStopSprint);

		// item interaction
		if (UInteractionComponent* InteractionComp = Interaction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, InteractionComp, &UInteractionComponent::Interact);
			EnhancedInputComponent->BindAction(CancelAction, ETriggerEvent::Started, InteractionComp, &UInteractionComponent::CancelInspect);
		}
	}
	else
	{
		UE_LOG(LogHorrorTemplate, Error, TEXT("'%s' failed to find an Enhanced Input Component. Sprint input will not work."), *GetNameSafe(this));
	}
}

void AHorrorPlayerPawn::DoAim(float Yaw, float Pitch)
{
	// while inspecting an item, look input spins the item instead of the player
	if (Interaction && Interaction->IsInspecting())
	{
		Interaction->AddInspectRotation(Yaw, Pitch);
		return;
	}

	Super::DoAim(Yaw, Pitch);
}

void AHorrorPlayerPawn::DoMove(float Right, float Forward)
{
	if (Interaction && Interaction->IsInspecting())
	{
		return;
	}

	Super::DoMove(Right, Forward);
}

void AHorrorPlayerPawn::DoJumpStart()
{
	if (Interaction && Interaction->IsInspecting())
	{
		return;
	}

	Super::DoJumpStart();
}

void AHorrorPlayerPawn::DoStartSprint()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = SprintSpeed;
	}
}

void AHorrorPlayerPawn::DoStopSprint()
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = WalkSpeed;
	}
}
