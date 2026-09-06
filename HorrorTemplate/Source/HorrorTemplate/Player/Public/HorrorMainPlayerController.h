// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorMainPlayerController.generated.h"

class UInputMappingContext;
class UInventoryComponent;

/**
 *  First person Player Controller for the Horror game.
 *  Registers the Enhanced Input mapping contexts for the local player.
 *  Owns the inventory component, which survives pawn death / respawn.
 *  Blueprint subclasses supply the actual UInputMappingContext assets.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AHorrorMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/** Constructor */
	AHorrorMainPlayerController();

protected:

	/** Input Mapping Contexts added for the local player during input setup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	/** Priority used when registering the mapping contexts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (ClampMin = 0))
	int32 InputMappingPriority = 0;

	/** Holds owned items and the items equipped in each hand */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> Inventory;

	/** Registers the Enhanced Input mapping contexts */
	virtual void SetupInputComponent() override;

	/** Rebuilds equipped item visuals on the newly possessed pawn */
	virtual void OnPossess(APawn* InPawn) override;

	/** Tears down equipped item visuals from the released pawn */
	virtual void OnUnPossess() override;

public:

	/** Returns the inventory component */
	UFUNCTION(BlueprintPure, Category="Inventory")
	UInventoryComponent* GetInventory() const { return Inventory; }
};
