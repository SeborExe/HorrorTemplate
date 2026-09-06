// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HorrorTemplateCharacter.h"
#include "HorrorPlayerPawn.generated.h"

class UInputAction;

/**
 *  First person player pawn for the Horror game.
 *  Builds on AHorrorTemplateCharacter: capsule, first person mesh, head-socket
 *  camera and the Enhanced Input move / look / jump bindings.
 *  Adds hold-to-sprint: the pawn walks at WalkSpeed by default and moves at
 *  SprintSpeed while the sprint action is held.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AHorrorPlayerPawn : public AHorrorTemplateCharacter
{
	GENERATED_BODY()

protected:

	/** Sprint Input Action. Bound to Started / Completed for hold-to-sprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> SprintAction;

	/** Ground speed while walking. Applied to MaxWalkSpeed on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta = (ClampMin = 0, Units = "cm/s"))
	float WalkSpeed = 150.0f;

	/** Ground speed while the sprint action is held */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta = (ClampMin = 0, Units = "cm/s"))
	float SprintSpeed = 400.0f;

public:

	/** Constructor */
	AHorrorPlayerPawn();

protected:

	/** Applies the default walk speed */
	virtual void BeginPlay() override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Raises the movement speed to SprintSpeed. Also callable from touch UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartSprint();

	/** Restores the movement speed to WalkSpeed. Also callable from touch UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopSprint();
};
