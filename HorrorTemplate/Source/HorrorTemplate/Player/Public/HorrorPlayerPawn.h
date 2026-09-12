// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HorrorTemplateCharacter.h"
#include "HorrorPlayerPawn.generated.h"

class UInputAction;
class UInteractionComponent;

/**
 *  First person player pawn for the Horror game.
 *  Builds on AHorrorTemplateCharacter: capsule, first person mesh, head-socket
 *  camera and the Enhanced Input move / look / jump bindings.
 *  Adds hold-to-sprint: the pawn walks at WalkSpeed by default and moves at
 *  SprintSpeed while the sprint action is held.
 *  Carries the interaction component, which drives item focus / inspection; look
 *  and movement input are suppressed here while an item is being inspected.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AHorrorPlayerPawn : public AHorrorTemplateCharacter
{
	GENERATED_BODY()

protected:

	/** Sprint Input Action. Bound to Started / Completed for hold-to-sprint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> SprintAction;

	/** Interact Input Action (E). Focus an item to inspect it, then again to pick it up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> InteractAction;

	/** Cancel Input Action (Esc). Puts an inspected item back */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> CancelAction;

	/** Use Left Hand Input Action (LMB). Activates UseItem on the left hand's item, or a two handed item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> UseLeftAction;

	/** Use Right Hand Input Action (RMB). Activates UseItem on the right hand's item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	TObjectPtr<UInputAction> UseRightAction;

	/** Ground speed while walking. Applied to MaxWalkSpeed on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta = (ClampMin = 0, Units = "cm/s"))
	float WalkSpeed = 150.0f;

	/** Ground speed while the sprint action is held */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta = (ClampMin = 0, Units = "cm/s"))
	float SprintSpeed = 400.0f;

	/** Drives item focus highlighting and inspection */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInteractionComponent> Interaction;

public:

	/** Constructor */
	AHorrorPlayerPawn();

	/** Returns the interaction component */
	UFUNCTION(BlueprintPure, Category="Interaction")
	UInteractionComponent* GetInteraction() const { return Interaction; }

protected:

	/** Applies the default walk speed */
	virtual void BeginPlay() override;

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/** Rotates the inspected item instead of the player while inspecting */
	virtual void DoAim(float Yaw, float Pitch) override;

	/** Suppressed while inspecting an item */
	virtual void DoMove(float Right, float Forward) override;

	/** Suppressed while inspecting an item */
	virtual void DoJumpStart() override;

	/** Raises the movement speed to SprintSpeed. Also callable from touch UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStartSprint();

	/** Restores the movement speed to WalkSpeed. Also callable from touch UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoStopSprint();

	/** Activates UseItem on the left hand's item (or a two handed item). Also callable from touch UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoUseLeftHand();

	/** Activates UseItem on the right hand's item. Also callable from touch UI */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoUseRightHand();
};
