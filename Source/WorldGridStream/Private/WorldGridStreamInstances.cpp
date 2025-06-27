// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridStreamInstances.h"
#include "WorldGridStreamInstancesActor.h"

BEGIN_FUNCTION_BUILD_OPTIMIZATION

UWorldGridStreamInstances::UWorldGridStreamInstances()
{

}

UWorldGridStreamInstances::~UWorldGridStreamInstances()
{

}


UWorldGridStreamInstances* UWorldGridStreamInstances::FindInstances(UWorld* InWorld, const FInt64Vector& InGridIndex)
{
	if(nullptr == InWorld)
	{
		return nullptr;
	}
	UWorldGridStreamInstances* WorldGridStreamInstances = nullptr;

	AWorldGridStreamInstancesActor* WorldGridStreamInstancesActor = AWorldGridStreamInstancesActor::GetWorldGridStreamInstancesActor(InWorld);
	WorldGridStreamInstances = WorldGridStreamInstancesActor->WorldGridStreamInstancesMap.FindRef(InGridIndex);
	return WorldGridStreamInstances;
}

UWorldGridStreamInstances* UWorldGridStreamInstances::FindOrCreateInstances(UWorld* InWorld, const FInt64Vector& InGridIndex)
{
	if(nullptr == InWorld)
	{
		return nullptr;
	}
	UWorldGridStreamInstances* WorldGridStreamInstances = nullptr;
	
	AWorldGridStreamInstancesActor* WorldGridStreamInstancesActor = AWorldGridStreamInstancesActor::GetWorldGridStreamInstancesActor(InWorld);
	WorldGridStreamInstances = WorldGridStreamInstancesActor->WorldGridStreamInstancesMap.FindRef(InGridIndex);

	if (nullptr == WorldGridStreamInstances)
	{
		const FString MapName = InWorld->GetMapName();
		const FString InstancesObjectName = FString::Printf(TEXT("%s_%s"), *MapName, *InGridIndex.ToString());
		const FString PackageName = FString::Printf(TEXT("/Game/WorldGridStream/%s"), *InstancesObjectName);
		UPackage* Package = CreatePackage(*PackageName);
		WorldGridStreamInstances = NewObject<UWorldGridStreamInstances>(Package, *InstancesObjectName, RF_Public | RF_Transactional | RF_Standalone);
		WorldGridStreamInstancesActor->Modify(false);
		WorldGridStreamInstancesActor->WorldGridStreamInstancesMap.Emplace(InGridIndex, WorldGridStreamInstances);
	}
	check(WorldGridStreamInstances);
	return WorldGridStreamInstances;
}

END_FUNCTION_BUILD_OPTIMIZATION