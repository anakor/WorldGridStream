// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridStreamInstancesActor.h"
#include "WorldGridStreamInstances.h"


AWorldGridStreamInstancesActor::AWorldGridStreamInstancesActor(class FObjectInitializer const& ObjectInitializer)
	:Super(ObjectInitializer)
{

}
AWorldGridStreamInstancesActor::~AWorldGridStreamInstancesActor()
{

}


void AWorldGridStreamInstancesActor::BeginDestroy()
{
	Super::BeginDestroy();

	if (World.IsValid())
	{
		World->PerModuleDataObjects.Remove(this);
	}
}

void AWorldGridStreamInstancesActor::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsTransacting() || Ar.IsObjectReferenceCollector())
	{
		Ar << WorldGridStreamInstancesMap;
	}
}

void AWorldGridStreamInstancesActor::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

    AWorldGridStreamInstancesActor* This = CastChecked<AWorldGridStreamInstancesActor>(InThis);
    for (auto WorldGridStreamInstancesPair : This->WorldGridStreamInstancesMap)
    {
        TObjectPtr<UWorldGridStreamInstances> WorldGridStreamInstances = WorldGridStreamInstancesPair.Value;
        for (AActor* Actor : WorldGridStreamInstances->WorldGridStreamActors)
        {
            if (Actor)
            {
                This->AddReferencedObjects(Actor, Collector);
            }
        }
    }
}

AWorldGridStreamInstancesActor* AWorldGridStreamInstancesActor::GetWorldGridStreamInstancesActor(const UWorld* World)
{
	AWorldGridStreamInstancesActor* FoundActor = nullptr;
	World->PerModuleDataObjects.FindItemByClass(&FoundActor);
	return FoundActor;
}