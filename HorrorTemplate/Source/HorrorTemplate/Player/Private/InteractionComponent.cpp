// Copyright Epic Games, Inc. All Rights Reserved.


#include "InteractionComponent.h"
#include "ItemBase.h"
#include "InventoryComponent.h"
#include "HorrorMainPlayerController.h"
#include "HorrorTemplateCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HorrorTemplate.h"

UInteractionComponent::UInteractionComponent()
{
	// focus runs on a timer, not a tick
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const AHorrorTemplateCharacter* Character = Cast<AHorrorTemplateCharacter>(GetOwner()))
	{
		CachedCamera = Character->GetFirstPersonCameraComponent();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FocusTimer, this, &UInteractionComponent::UpdateFocus, FocusCheckInterval, true);
	}
}

void UInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FocusTimer);
	}

	// don't leave an item attached to a pawn that's going away
	if (bIsInspecting)
	{
		CancelInspect();
	}
	else
	{
		SetFocusedItem(nullptr);
	}

	Super::EndPlay(EndPlayReason);
}

void UInteractionComponent::Interact()
{
	if (!bIsInspecting)
	{
		BeginInspect();
		return;
	}

	// while inspecting, Interact picks the item up - but only if it can be picked up.
	// View-only items are left in inspection; the player exits with Cancel.
	if (InspectedItem && InspectedItem->CanBePickedUp())
	{
		ConfirmPickup();
	}
	else
	{
		UE_LOG(LogHorrorTemplate, Warning, TEXT("InteractionComponent: '%s' cannot be picked up; use Cancel to stop inspecting"), *GetNameSafe(InspectedItem));
	}
}

void UInteractionComponent::UpdateFocus()
{
	if (bIsInspecting)
	{
		return;
	}

	UCameraComponent* Camera = ResolveCamera();
	UWorld* World = GetWorld();

	if (!Camera || !World)
	{
		return;
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * TraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	FHitResult Hit;
	World->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, QueryParams);

	AItemBase* HitItem = Cast<AItemBase>(Hit.GetActor());

	// only items the player can actually interact with get focused / highlighted
	if (HitItem && !HitItem->CanBeInspected())
	{
		HitItem = nullptr;
	}

	SetFocusedItem(HitItem);
}

void UInteractionComponent::SetFocusedItem(AItemBase* NewItem)
{
	if (FocusedItem == NewItem)
	{
		return;
	}

	if (FocusedItem)
	{
		FocusedItem->Unhighlight();
	}

	FocusedItem = NewItem;

	if (FocusedItem)
	{
		FocusedItem->Highlight();
	}

	OnFocusedItemChanged.Broadcast(FocusedItem);
}

void UInteractionComponent::BeginInspect()
{
	UCameraComponent* Camera = ResolveCamera();

	if (bIsInspecting || !FocusedItem || !Camera)
	{
		return;
	}

	AItemBase* Item = FocusedItem;

	// stop focusing / highlighting the item while it's being inspected
	SetFocusedItem(nullptr);

	// remember how to put it back
	InspectRestoreTransform = Item->GetActorTransform();
	bInspectRestoreCollision = Item->GetActorEnableCollision();

	Item->SetActorEnableCollision(false);

	// snap location / rotation to the camera but keep the item's own world scale
	const FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false);
	Item->AttachToComponent(Camera, AttachRules);
	Item->SetActorRelativeLocation(FVector(InspectDistance, 0.0f, 0.0f));
	Item->SetActorRelativeRotation(FRotator::ZeroRotator);

	InspectedItem = Item;
	bIsInspecting = true;

	OnInspectStateChanged.Broadcast(true);

	if (UInventoryComponent* Inventory = GetInventory())
	{
		Inventory->NotifyInspectionStarted(Item);
	}
}

void UInteractionComponent::ConfirmPickup()
{
	if (!bIsInspecting || !InspectedItem)
	{
		return;
	}

	AItemBase* Item = InspectedItem;

	InspectedItem = nullptr;
	bIsInspecting = false;

	if (UInventoryComponent* Inventory = GetInventory())
	{
		const TSubclassOf<AItemBase> ItemClass = Item->GetClass();

		Inventory->AddItem(ItemClass);

		// equippable items drop straight into a free matching hand
		Inventory->AutoEquip(ItemClass);

		Inventory->NotifyInspectionEnded();
	}
	else
	{
		UE_LOG(LogHorrorTemplate, Warning, TEXT("InteractionComponent: no inventory found, item picked up but not stored"));
	}

	Item->Destroy();

	OnInspectStateChanged.Broadcast(false);
}

void UInteractionComponent::CancelInspect()
{
	if (!bIsInspecting || !InspectedItem)
	{
		return;
	}

	AItemBase* Item = InspectedItem;

	InspectedItem = nullptr;
	bIsInspecting = false;

	Item->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	Item->SetActorTransform(InspectRestoreTransform);
	Item->SetActorEnableCollision(bInspectRestoreCollision);

	OnInspectStateChanged.Broadcast(false);

	if (UInventoryComponent* Inventory = GetInventory())
	{
		Inventory->NotifyInspectionEnded();
	}
}

void UInteractionComponent::AddInspectRotation(float Yaw, float Pitch)
{
	UCameraComponent* Camera = ResolveCamera();

	if (!bIsInspecting || !InspectedItem || !Camera)
	{
		return;
	}

	// rotate about the camera axes so dragging feels the same whatever way the item faces
	const FVector UpAxis = Camera->GetUpVector();
	const FVector RightAxis = Camera->GetRightVector();

	InspectedItem->AddActorWorldRotation(FQuat(UpAxis, FMath::DegreesToRadians(-Yaw * InspectRotationSpeed)));
	InspectedItem->AddActorWorldRotation(FQuat(RightAxis, FMath::DegreesToRadians(Pitch * InspectRotationSpeed)));
}

UCameraComponent* UInteractionComponent::ResolveCamera()
{
	if (CachedCamera)
	{
		return CachedCamera;
	}

	if (const AHorrorTemplateCharacter* Character = Cast<AHorrorTemplateCharacter>(GetOwner()))
	{
		CachedCamera = Character->GetFirstPersonCameraComponent();
	}

	return CachedCamera;
}

UInventoryComponent* UInteractionComponent::GetInventory() const
{
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		if (const AHorrorMainPlayerController* PlayerController = Cast<AHorrorMainPlayerController>(OwnerPawn->GetController()))
		{
			return PlayerController->GetInventory();
		}
	}

	return nullptr;
}
