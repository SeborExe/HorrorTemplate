// Copyright Epic Games, Inc. All Rights Reserved.


#include "InventoryComponent.h"
#include "ItemBase.h"
#include "ItemEquippable.h"
#include "HorrorTemplateCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "HorrorTemplate.h"

UInventoryComponent::UInventoryComponent()
{
	// purely event driven
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(TSubclassOf<AItemBase> ItemClass)
{
	if (!ItemClass)
	{
		return false;
	}

	OwnedItems.Add(ItemClass);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(TSubclassOf<AItemBase> ItemClass)
{
	if (!ItemClass)
	{
		return false;
	}

	const int32 RemoveIndex = OwnedItems.Find(ItemClass);
	if (RemoveIndex == INDEX_NONE)
	{
		return false;
	}

	OwnedItems.RemoveAt(RemoveIndex);

	// only unequip when the owner no longer has any copy of this item
	if (!OwnedItems.Contains(ItemClass))
	{
		if (EquippedRightClass == ItemClass || EquippedLeftClass == ItemClass)
		{
			// UnequipHand handles the two handed case (clears both slots)
			UnequipHand(EquippedRightClass == ItemClass ? EItemEquipHand::Right : EItemEquipHand::Left);
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::HasItem(TSubclassOf<AItemBase> ItemClass) const
{
	return ItemClass && OwnedItems.Contains(ItemClass);
}

bool UInventoryComponent::EquipToHand(TSubclassOf<AItemBase> ItemClass, EItemEquipHand Hand)
{
	if (!ItemClass || !HasItem(ItemClass))
	{
		UE_LOG(LogHorrorTemplate, Warning, TEXT("EquipToHand: item is null or not owned"));
		return false;
	}

	if (const AItemBase* DefaultItem = ItemClass.GetDefaultObject())
	{
		if (!DefaultItem->CanBeEquipped())
		{
			UE_LOG(LogHorrorTemplate, Warning, TEXT("EquipToHand: '%s' is not equippable"), *GetNameSafe(*ItemClass));
			return false;
		}
	}

	bool bTwoHanded = false;
	EItemEquipHand AllowedHand = EItemEquipHand::Any;
	GetItemDefaults(ItemClass, bTwoHanded, AllowedHand);

	// two handed: ignore the requested hand, take both slots with a single instance
	if (bTwoHanded)
	{
		UnequipAll();

		AItemBase* SpawnedItem = SpawnAndAttach(ItemClass, ResolveAttachSocket(ItemClass, EItemEquipHand::Right));
		if (!SpawnedItem)
		{
			return false;
		}

		EquippedRightClass = ItemClass;
		EquippedLeftClass = ItemClass;
		EquippedRightActor = SpawnedItem;
		EquippedLeftActor = nullptr;
		bTwoHandedEquipped = true;

		OnEquippedChanged.Broadcast(EItemEquipHand::Right, SpawnedItem);
		OnEquippedChanged.Broadcast(EItemEquipHand::Left, SpawnedItem);
		return true;
	}

	// one handed: resolve which hand to use
	EItemEquipHand TargetHand = Hand;

	if (AllowedHand == EItemEquipHand::Left || AllowedHand == EItemEquipHand::Right)
	{
		// the item restricts itself to a specific hand
		TargetHand = AllowedHand;
	}
	else if (TargetHand == EItemEquipHand::Any)
	{
		// nobody cares which hand: prefer the right, fall back to the left if only the right is taken
		TargetHand = (IsHandOccupied(EItemEquipHand::Right) && !IsHandOccupied(EItemEquipHand::Left))
			? EItemEquipHand::Left
			: EItemEquipHand::Right;
	}

	if (TargetHand != EItemEquipHand::Left && TargetHand != EItemEquipHand::Right)
	{
		return false;
	}

	// free the target hand (also clears a two handed item spanning both slots)
	UnequipHand(TargetHand);

	AItemBase* SpawnedItem = SpawnAndAttach(ItemClass, ResolveAttachSocket(ItemClass, TargetHand));
	if (!SpawnedItem)
	{
		return false;
	}

	if (TargetHand == EItemEquipHand::Right)
	{
		EquippedRightClass = ItemClass;
		EquippedRightActor = SpawnedItem;
	}
	else
	{
		EquippedLeftClass = ItemClass;
		EquippedLeftActor = SpawnedItem;
	}

	OnEquippedChanged.Broadcast(TargetHand, SpawnedItem);
	return true;
}

bool UInventoryComponent::AutoEquip(TSubclassOf<AItemBase> ItemClass)
{
	if (!ItemClass)
	{
		return false;
	}

	const AItemBase* DefaultItem = ItemClass.GetDefaultObject();
	if (!DefaultItem || !DefaultItem->CanBeEquipped())
	{
		return false;
	}

	const FItemData& Data = DefaultItem->GetItemData();

	// two handed only auto equips when both hands are empty
	if (Data.bRequiresBothHands)
	{
		if (IsHandOccupied(EItemEquipHand::Right) || IsHandOccupied(EItemEquipHand::Left))
		{
			return false;
		}

		return EquipToHand(ItemClass, EItemEquipHand::Any);
	}

	// one handed: only auto equip if the hand it wants is free
	switch (Data.EquipHand)
	{
	case EItemEquipHand::Right:
		return !IsHandOccupied(EItemEquipHand::Right) && EquipToHand(ItemClass, EItemEquipHand::Right);

	case EItemEquipHand::Left:
		return !IsHandOccupied(EItemEquipHand::Left) && EquipToHand(ItemClass, EItemEquipHand::Left);

	default: // Any: prefer the right hand, then the left
		if (!IsHandOccupied(EItemEquipHand::Right))
		{
			return EquipToHand(ItemClass, EItemEquipHand::Right);
		}
		if (!IsHandOccupied(EItemEquipHand::Left))
		{
			return EquipToHand(ItemClass, EItemEquipHand::Left);
		}
		return false;
	}
}

void UInventoryComponent::UnequipHand(EItemEquipHand Hand)
{
	// a two handed item spans both slots, so any unequip drops the whole thing
	if (bTwoHandedEquipped)
	{
		if (EquippedRightActor)
		{
			EquippedRightActor->Destroy();
		}

		EquippedRightActor = nullptr;
		EquippedLeftActor = nullptr;
		EquippedRightClass = nullptr;
		EquippedLeftClass = nullptr;
		bTwoHandedEquipped = false;

		OnEquippedChanged.Broadcast(EItemEquipHand::Right, nullptr);
		OnEquippedChanged.Broadcast(EItemEquipHand::Left, nullptr);
		return;
	}

	if (Hand == EItemEquipHand::Right || Hand == EItemEquipHand::Any)
	{
		ClearHand(EItemEquipHand::Right);
	}

	if (Hand == EItemEquipHand::Left || Hand == EItemEquipHand::Any)
	{
		ClearHand(EItemEquipHand::Left);
	}
}

void UInventoryComponent::UnequipAll()
{
	if (bTwoHandedEquipped)
	{
		// clears both slots
		UnequipHand(EItemEquipHand::Right);
		return;
	}

	ClearHand(EItemEquipHand::Right);
	ClearHand(EItemEquipHand::Left);
}

void UInventoryComponent::HandlePawnChanged(APawn* NewPawn)
{
	// tear down visuals attached to the previous pawn, keep the class records
	if (EquippedRightActor)
	{
		EquippedRightActor->Destroy();
		EquippedRightActor = nullptr;
	}

	if (EquippedLeftActor)
	{
		EquippedLeftActor->Destroy();
		EquippedLeftActor = nullptr;
	}

	if (!NewPawn)
	{
		return;
	}

	// rebuild the visuals from the surviving class records
	if (bTwoHandedEquipped && EquippedRightClass)
	{
		EquippedRightActor = SpawnAndAttach(EquippedRightClass, ResolveAttachSocket(EquippedRightClass, EItemEquipHand::Right));
		OnEquippedChanged.Broadcast(EItemEquipHand::Right, EquippedRightActor);
		OnEquippedChanged.Broadcast(EItemEquipHand::Left, EquippedRightActor);
		return;
	}

	if (EquippedRightClass)
	{
		EquippedRightActor = SpawnAndAttach(EquippedRightClass, ResolveAttachSocket(EquippedRightClass, EItemEquipHand::Right));
		OnEquippedChanged.Broadcast(EItemEquipHand::Right, EquippedRightActor);
	}

	if (EquippedLeftClass)
	{
		EquippedLeftActor = SpawnAndAttach(EquippedLeftClass, ResolveAttachSocket(EquippedLeftClass, EItemEquipHand::Left));
		OnEquippedChanged.Broadcast(EItemEquipHand::Left, EquippedLeftActor);
	}
}

void UInventoryComponent::NotifyInspectionStarted(AItemBase* Item)
{
	OnInspectionStarted.Broadcast(Item);
}

void UInventoryComponent::NotifyInspectionEnded()
{
	OnInspectionEnded.Broadcast();
}

AItemBase* UInventoryComponent::GetEquippedItem(EItemEquipHand Hand) const
{
	switch (Hand)
	{
	case EItemEquipHand::Right:
		return EquippedRightActor;

	case EItemEquipHand::Left:
		return EquippedLeftActor;

	default:
		return EquippedRightActor ? EquippedRightActor : EquippedLeftActor;
	}
}

bool UInventoryComponent::IsHandOccupied(EItemEquipHand Hand) const
{
	switch (Hand)
	{
	case EItemEquipHand::Right:
		return EquippedRightClass != nullptr;

	case EItemEquipHand::Left:
		return EquippedLeftClass != nullptr;

	default:
		return EquippedRightClass != nullptr || EquippedLeftClass != nullptr;
	}
}

USkeletalMeshComponent* UInventoryComponent::GetAttachMesh() const
{
	// the component lives on the controller; the hand sockets are on the possessed pawn
	APawn* ControlledPawn = nullptr;
	if (const AController* OwningController = Cast<AController>(GetOwner()))
	{
		ControlledPawn = OwningController->GetPawn();
	}
	else
	{
		ControlledPawn = Cast<APawn>(GetOwner());
	}

	if (const AHorrorTemplateCharacter* Character = Cast<AHorrorTemplateCharacter>(ControlledPawn))
	{
		return Character->GetFirstPersonMesh();
	}

	return nullptr;
}

bool UInventoryComponent::GetItemDefaults(TSubclassOf<AItemBase> ItemClass, bool& bOutTwoHanded, EItemEquipHand& OutHand)
{
	bOutTwoHanded = false;
	OutHand = EItemEquipHand::Any;

	if (!ItemClass)
	{
		return false;
	}

	const AItemBase* DefaultItem = ItemClass.GetDefaultObject();
	if (!DefaultItem)
	{
		return false;
	}

	const FItemData& Data = DefaultItem->GetItemData();
	bOutTwoHanded = Data.bRequiresBothHands;
	OutHand = Data.EquipHand;
	return true;
}

FName UInventoryComponent::ResolveAttachSocket(TSubclassOf<AItemBase> ItemClass, EItemEquipHand Hand) const
{
	// item defined socket wins, so designers control exactly how it sits in the hand
	if (const AItemEquippable* Equippable = Cast<AItemEquippable>(ItemClass.GetDefaultObject()))
	{
		if (!Equippable->GetSocketName().IsNone())
		{
			return Equippable->GetSocketName();
		}
	}

	return (Hand == EItemEquipHand::Left) ? LeftHandSocketName : RightHandSocketName;
}

AItemBase* UInventoryComponent::SpawnAndAttach(TSubclassOf<AItemBase> ItemClass, FName SocketName)
{
	USkeletalMeshComponent* AttachMesh = GetAttachMesh();
	UWorld* World = GetWorld();

	if (!ItemClass || !AttachMesh || !World)
	{
		UE_LOG(LogHorrorTemplate, Warning, TEXT("SpawnAndAttach: missing item class, attach mesh or world"));
		return nullptr;
	}

	APawn* OwningPawn = Cast<APawn>(AttachMesh->GetOwner());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningPawn;
	SpawnParams.Instigator = OwningPawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AItemBase* SpawnedItem = World->SpawnActor<AItemBase>(ItemClass, AttachMesh->GetSocketTransform(SocketName), SpawnParams);
	if (!SpawnedItem)
	{
		return nullptr;
	}

	// held items don't collide with the world; per item attach transform / collision
	// policy comes with the AItemBase extension
	SpawnedItem->SetActorEnableCollision(false);

	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);
	SpawnedItem->AttachToComponent(AttachMesh, AttachmentRule, SocketName);

	// equippables define how big they sit in the hand
	if (const AItemEquippable* Equippable = Cast<AItemEquippable>(ItemClass.GetDefaultObject()))
	{
		SpawnedItem->SetActorRelativeScale3D(Equippable->GetHandScale());
	}

	return SpawnedItem;
}

void UInventoryComponent::ClearHand(EItemEquipHand Hand)
{
	// Right / Left only; the two handed case is handled in UnequipHand
	const bool bRight = (Hand == EItemEquipHand::Right);

	TObjectPtr<AItemBase>& SlotActor = bRight ? EquippedRightActor : EquippedLeftActor;
	TSubclassOf<AItemBase>& SlotClass = bRight ? EquippedRightClass : EquippedLeftClass;

	const bool bWasOccupied = (SlotActor != nullptr) || (SlotClass != nullptr);

	if (SlotActor)
	{
		SlotActor->Destroy();
	}

	SlotActor = nullptr;
	SlotClass = nullptr;

	if (bWasOccupied)
	{
		OnEquippedChanged.Broadcast(Hand, nullptr);
	}
}
