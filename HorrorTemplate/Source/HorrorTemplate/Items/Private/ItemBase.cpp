// Copyright Epic Games, Inc. All Rights Reserved.


#include "ItemBase.h"
#include "Components/StaticMeshComponent.h"

AItemBase::AItemBase()
{
	// items are event driven, no per-frame work
	PrimaryActorTick.bCanEverTick = false;

	// the mesh is the root of the item
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();

	// set up the outline stencil once; the outline itself is toggled in Highlight/Unhighlight
	Mesh->SetCustomDepthStencilValue(HighlightStencilValue);
	Mesh->SetRenderCustomDepth(false);
}

void AItemBase::Highlight()
{
	if (bIsHighlighted)
	{
		return;
	}

	bIsHighlighted = true;

	Mesh->SetRenderCustomDepth(true);

	BP_OnHighlightChanged(true);
}

void AItemBase::Unhighlight()
{
	if (!bIsHighlighted)
	{
		return;
	}

	bIsHighlighted = false;

	Mesh->SetRenderCustomDepth(false);

	BP_OnHighlightChanged(false);
}
