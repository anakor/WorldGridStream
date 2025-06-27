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
	/* * ���� �׸��� ��Ʈ�� ���͸� �����ϴ� �迭
	 * ���� �׸��� ��Ʈ�� ���ʹ� ���� �׸��� ��Ʈ���� �����ϴ� ���͵��, 
	 * �� �迭�� �ش� ���͵��� �����ϰ� ��ȯ�ϴ� �� ���
	 * �� ���͵��� World�� ��ġ�Ǿ� �ִ� �͵� ��
	 */
	UPROPERTY(VisibleAnywhere, Category = "WorldGridStream|Actor")
	TArray<AActor*> WorldGridStreamActors;
	
	/* * uint64�� Actor�� Name�� Hash�� ��
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
