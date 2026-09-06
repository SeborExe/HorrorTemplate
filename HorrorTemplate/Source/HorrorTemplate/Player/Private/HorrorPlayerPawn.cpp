// Copyright Epic Games, Inc. All Rights Reserved.


#include "HorrorPlayerPawn.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HorrorTemplate.h"

AHorrorPlayerPawn::AHorrorPlayerPawn()
{
	// movement is driven by input events and the character movement component's
	// own tick, so the pawn actor itself doesn't need to tick
	PrimaryActorTick.bCanEverTick = false;
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
	}
	else
	{
		UE_LOG(LogHorrorTemplate, Error, TEXT("'%s' failed to find an Enhanced Input Component. Sprint input will not work."), *GetNameSafe(this));
	}
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
