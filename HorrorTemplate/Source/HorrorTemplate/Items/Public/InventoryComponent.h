// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemTypes.h"
#include "InventoryComponent.generated.h"

class AItemBase;
class APawn;
class USkeletalMeshComponent;

/** Broadcast when the owned item list changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInventoryChangedDelegate);

/** Broadcast when a hand slot is equipped or cleared. Item is null when cleared */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquippedChangedDelegate, EItemEquipHand, Hand, AItemBase*, Item);

/** Broadcast when the player starts inspecting a world item */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInspectionStartedDelegate, AItemBase*, Item);

/** Broadcast when the player stops inspecting a world item (picked up or put back) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInspectionEndedDelegate);

/**
 *  Inventory for a first person character, meant to live on the Player Controller
 *  so it survives pawn death / respawn.
 *  Tracks the owned item classes plus the item equipped in each hand. A two handed
 *  item (FItemData::bRequiresBothHands) occupies both hand slots. Equipped items are
 *  spawned as AItemBase actors and attached to the pawn's first person mesh at the
 *  hand_r / hand_l sockets.
 */
UCLASS(ClassGroup=(Horror), meta=(BlueprintSpawnableComponent))
class HORRORTEMPLATE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	/** Constructor */
	UInventoryComponent();

	/** Broadcast when OwnedItems changes */
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FInventoryChangedDelegate OnInventoryChanged;

	/** Broadcast when a hand slot is equipped or cleared */
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FEquippedChangedDelegate OnEquippedChanged;

	/** Broadcast when item inspection starts. Forwarded from the interaction component */
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FInspectionStartedDelegate OnInspectionStarted;

	/** Broadcast when item inspection ends. Forwarded from the interaction component */
	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FInspectionEndedDelegate OnInspectionEnded;

protected:

	/** Classes of every item the owner currently holds. Duplicates are allowed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TArray<TSubclassOf<AItemBase>> OwnedItems;

	/** Item class equipped in the right hand, if any. Survives pawn changes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TSubclassOf<AItemBase> EquippedRightClass;

	/** Item class equipped in the left hand, if any. Survives pawn changes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	TSubclassOf<AItemBase> EquippedLeftClass;

	/** Spawned instance held in the right hand. Recreated per possessed pawn */
	UPROPERTY(Transient)
	TObjectPtr<AItemBase> EquippedRightActor;

	/** Spawned instance held in the left hand. Recreated per possessed pawn */
	UPROPERTY(Transient)
	TObjectPtr<AItemBase> EquippedLeftActor;

	/** True while a two handed item occupies both hand slots */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Inventory")
	bool bTwoHandedEquipped = false;

	/** Socket / bone on the first person mesh the right hand item attaches to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FName RightHandSocketName = FName("hand_r");

	/** Socket / bone on the first person mesh the left hand item attaches to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FName LeftHandSocketName = FName("hand_l");

public:

	/** Adds an item class to the owned list */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool AddItem(TSubclassOf<AItemBase> ItemClass);

	/** Removes one copy of an item class from the owned list. Unequips it if that was the last copy */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool RemoveItem(TSubclassOf<AItemBase> ItemClass);

	/** Returns true if the owned list contains the item class */
	UFUNCTION(BlueprintPure, Category="Inventory")
	bool HasItem(TSubclassOf<AItemBase> ItemClass) const;

	/**
	 *  Equips an owned item into a hand and spawns its visual on the pawn.
	 *  A two handed item ignores Hand and takes both slots.
	 *  Hand == Any picks a free hand, honoring the item's own EquipHand restriction.
	 *  Any item already in the target hand(s) is unequipped first.
	 */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool EquipToHand(TSubclassOf<AItemBase> ItemClass, EItemEquipHand Hand);

	/** Clears a hand slot and destroys its visual. Clears both slots for a two handed item */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void UnequipHand(EItemEquipHand Hand);

	/** Clears both hand slots */
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void UnequipAll();

	/** Destroys the equipped visuals and rebuilds them on NewPawn. Call from the controller on possess / unpossess */
	void HandlePawnChanged(APawn* NewPawn);

	/** Fires OnInspectionStarted. Called by the interaction component when the player begins inspecting Item */
	void NotifyInspectionStarted(AItemBase* Item);

	/** Fires OnInspectionEnded. Called by the interaction component when the player stops inspecting */
	void NotifyInspectionEnded();

	/** Returns the owned item classes */
	UFUNCTION(BlueprintPure, Category="Inventory")
	TArray<TSubclassOf<AItemBase>> GetOwnedItems() const { return OwnedItems; }

	/** Returns the spawned item instance held in the given hand, or null */
	UFUNCTION(BlueprintPure, Category="Inventory")
	AItemBase* GetEquippedItem(EItemEquipHand Hand) const;

	/** Returns true if the given hand is occupied (Any = either hand) */
	UFUNCTION(BlueprintPure, Category="Inventory")
	bool IsHandOccupied(EItemEquipHand Hand) const;

	/** Returns true if a two handed item occupies both hands */
	UFUNCTION(BlueprintPure, Category="Inventory")
	bool IsTwoHandedEquipped() const { return bTwoHandedEquipped; }

protected:

	/** Resolves the possessed pawn's first person mesh */
	USkeletalMeshComponent* GetAttachMesh() const;

	/** Reads bRequiresBothHands / EquipHand from the item class default object */
	static bool GetItemDefaults(TSubclassOf<AItemBase> ItemClass, bool& bOutTwoHanded, EItemEquipHand& OutHand);

	/** Spawns an item actor and attaches it to the given socket on the attach mesh */
	AItemBase* SpawnAndAttach(TSubclassOf<AItemBase> ItemClass, FName SocketName);

	/** Destroys the visual and clears the class record for a single hand (Right or Left only) */
	void ClearHand(EItemEquipHand Hand);
};
