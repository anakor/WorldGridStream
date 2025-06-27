// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WorldGridStreamInstancesActor.generated.h"


UCLASS(ClassGroup=(Custom), hidecategories=(Actor, Display, Advanced, Physics, Networking, Replication, Rendering, HLOD, Collision, Input, DataLayers, LevelInstance, WorldPartition, Transform, Cooking), NotPlaceable, NotBlueprintable, MinimalAPI, meta = (DisplayName = "World Grid Stream Instances Actor"))
class AWorldGridStreamInstancesActor : public AActor
{
	GENERATED_BODY()

	friend class UWorldGridStreamInstances;
// Variables
public:
protected:
	UPROPERTY(Transient, VisibleAnywhere, Category = "World Grid Stream Instances")
	TMap<FInt64Vector, TObjectPtr<class UWorldGridStreamInstances>> WorldGridStreamInstancesMap;

	TWeakObjectPtr<UWorld> World;
private:

//Functions
public:
	AWorldGridStreamInstancesActor(class FObjectInitializer const& ObjectInitializer);
	virtual ~AWorldGridStreamInstancesActor();

	void BeginDestroy() override;
	void Serialize(FArchive& Ar) override;

	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	
	static AWorldGridStreamInstancesActor* GetWorldGridStreamInstancesActor(const UWorld* World);

	void SetWorld(UWorld* InWorld)
	{
		World = MakeWeakObjectPtr(InWorld);
	}
protected:
private:
};
