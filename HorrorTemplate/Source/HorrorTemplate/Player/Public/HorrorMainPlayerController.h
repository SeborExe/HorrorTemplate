// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorMainPlayerController.generated.h"

class UInputMappingContext;

/**
 *  First person Player Controller for the Horror game.
 *  Registers the Enhanced Input mapping contexts for the local player.
 *  Blueprint subclasses supply the actual UInputMappingContext assets.
 */
UCLASS(abstract)
class HORRORTEMPLATE_API AHorrorMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	/** Input Mapping Contexts added for the local player during input setup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	/** Priority used when registering the mapping contexts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta = (ClampMin = 0))
	int32 InputMappingPriority = 0;

	/** Registers the Enhanced Input mapping contexts */
	virtual void SetupInputComponent() override;
};
