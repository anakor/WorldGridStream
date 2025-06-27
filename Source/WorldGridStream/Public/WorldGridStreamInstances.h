// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
//#include "Containers/Map.h"
#include "WorldGridStreamInstances.generated.h"


UCLASS(ClassGroup=(Custom), hidecategories=(Actor, Display, Advanced, Physics, Networking, Replication, LevelInstance, WorldPartition, Transform, Cooking), NotBlueprintable, MinimalAPI, meta = (DisplayName = "World Grid Stream Instances"))
class UWorldGridStreamInstances : public UObject
{
	GENERATED_BODY()

// Variables
public:
	/* * 월드 그리드 스트림 액터를 저장하는 배열
	 * 월드 그리드 스트림 액터는 월드 그리드 스트림을 구성하는 액터들로, 
	 * 이 배열은 해당 액터들을 관리하고 변환하는 데 사용
	 * 이 액터들은 World에 배치되어 있던 것들 임
	 */
	UPROPERTY(VisibleAnywhere, Category = "WorldGridStream|Actor")
	TArray<AActor*> WorldGridStreamActors;
	
	/* * uint64는 Actor의 Name을 Hash한 값
	 */
	UPROPERTY()
	TMap<uint64, UClass*> ActorClassMaps;

protected:
private:

//Functions
public:
	UWorldGridStreamInstances();
	virtual ~UWorldGridStreamInstances();
	

	static UWorldGridStreamInstances* FindInstances(UWorld* InWorld, const FInt64Vector& GridIndex);
	static UWorldGridStreamInstances* FindOrCreateInstances(UWorld* InWorld, const FInt64Vector& GridIndex);
protected:
private:
};
