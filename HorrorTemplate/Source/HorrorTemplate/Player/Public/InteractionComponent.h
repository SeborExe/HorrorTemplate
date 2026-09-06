// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "InteractionComponent.generated.h"

class AItemBase;
class APawn;
class UCameraComponent;
class UInventoryComponent;

/** Broadcast when the item under the crosshair changes. Item is null when nothing is focused */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFocusedItemChangedDelegate, AItemBase*, Item);

/** Broadcast when the player enters or leaves item inspection */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInspectStateChangedDelegate, bool, bInspecting);

/**
 *  First person interaction, meant to live on the player pawn.
 *  Runs a periodic line trace from the camera; the AItemBase under the crosshair
 *  is highlighted (AItemBase::Highlight / Unhighlight).
 *  Interact (E) with a focused item lifts it in front of the camera for inspection;
 *  while inspecting, look input rotates the item instead of the player.
 *  Interact again picks the item up into the inventory; Cancel (Esc) puts it back.
 */
UCLASS(ClassGroup=(Horror), meta=(BlueprintSpawnableComponent))
class HORRORTEMPLATE_API UInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Constructor */
	UInteractionComponent();

	/** Broadcast when the focused item changes */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FFocusedItemChangedDelegate OnFocusedItemChanged;

	/** Broadcast when inspection starts or ends */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FInspectStateChangedDelegate OnInspectStateChanged;

protected:

	/** Seconds between focus traces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta = (ClampMin = 0.01, ClampMax = 1, Units = "s"))
	float FocusCheckInterval = 0.05f;

	/** Reach of the focus trace from the camera */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta = (ClampMin = 0, ClampMax = 2000, Units = "cm"))
	float TraceDistance = 250.0f;

	/** Collision channel used by the focus trace */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	/** Distance in front of the camera the item sits at while inspected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta = (ClampMin = 0, ClampMax = 200, Units = "cm"))
	float InspectDistance = 40.0f;

	/** Degrees of item rotation per unit of look input while inspecting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction", meta = (ClampMin = 0, ClampMax = 20))
	float InspectRotationSpeed = 1.0f;

	/** Camera the trace originates from. Resolved on BeginPlay */
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> CachedCamera;

	/** Item currently under the crosshair, if any */
	UPROPERTY(Transient)
	TObjectPtr<AItemBase> FocusedItem;

	/** Item currently held in front of the camera for inspection, if any */
	UPROPERTY(Transient)
	TObjectPtr<AItemBase> InspectedItem;

	/** True while inspecting an item */
	bool bIsInspecting = false;

	/** World transform to restore the inspected item to on cancel */
	FTransform InspectRestoreTransform;

	/** Collision enabled state to restore the inspected item to on cancel */
	bool bInspectRestoreCollision = true;

	/** Handle for the periodic focus trace */
	FTimerHandle FocusTimer;

public:

	/** Context action bound to the Interact key. Focused item -> inspect; inspecting -> pick up */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void Interact();

	/** Puts the inspected item back where it was and leaves inspection */
	UFUNCTION(BlueprintCallable, Category="Interaction")
	void CancelInspect();

	/** Rotates the inspected item from look input. No-op when not inspecting */
	void AddInspectRotation(float Yaw, float Pitch);

	/** Returns true while an item is being inspected */
	UFUNCTION(BlueprintPure, Category="Interaction")
	bool IsInspecting() const { return bIsInspecting; }

	/** Returns the item under the crosshair, or null */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AItemBase* GetFocusedItem() const { return FocusedItem; }

	/** Returns the item being inspected, or null */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AItemBase* GetInspectedItem() const { return InspectedItem; }

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Periodic trace from the camera that updates the focused item */
	void UpdateFocus();

	/** Swaps the focused item, driving Highlight / Unhighlight and the delegate */
	void SetFocusedItem(AItemBase* NewItem);

	/** Lifts the focused item in front of the camera */
	void BeginInspect();

	/** Moves the inspected item into the inventory and destroys the world actor */
	void ConfirmPickup();

	/** Returns the pawn's first person camera */
	UCameraComponent* ResolveCamera();

	/** Returns the inventory on the owning player controller, or null */
	UInventoryComponent* GetInventory() const;
};
