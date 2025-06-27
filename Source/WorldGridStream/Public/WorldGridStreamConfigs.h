// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "WorldGridStreamSettings.h" // AWorldGridStreamSettings

#include "WorldGridStreamConfigs.generated.h"

UCLASS(ClassGroup=(Custom), config = WorldGridStreamConfigs, DefaultConfig, hidecategories=(Actor, Display, Advanced, Physics, Networking, Replication, LevelInstance, WorldPartition, Transform, Cooking), NotBlueprintable, MinimalAPI, meta = (DisplayName = "World Grid Stream Settings"))
class UWorldGridStreamConfigs : public UDeveloperSettings
{
	GENERATED_BODY()

public:
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(config, EditAnywhere, Category = "World Grid Stream", meta=(DisplayName="Streaming Black List Classes"))
	TArray<TObjectPtr<UClass>> StreamingBlackListClasses; //Streaming�� ���� ���� Class���� �����ϴ� �迭.
	
	UPROPERTY(config, EditAnywhere, Category = "World Grid Stream", meta=(DisplayName="Streaming White List Classes"))
	TArray<TObjectPtr<UClass>> StreamingWhiteListClasses; //Streaming�� �� Class���� �����ϴ� �迭. White List�� �ִ� Class�� Black List�� �ִ� Class�� �����ϰ� Streaming�� �Ѵ�.
#endif // WITH_EDITORONLY_DATA
private:

public:
	UWorldGridStreamConfigs()
		: Super()
	{
#if WITH_EDITORONLY_DATA
		StreamingBlackListClasses.Emplace(AWorldGridStreamSettings::StaticClass());
#endif // WITH_EDITORONLY_DATA
	}
#if WITH_EDITOR
	const TArray<TObjectPtr<UClass>>& GetStreamingBlackListClasses() const { return StreamingBlackListClasses; }
	const TArray<TObjectPtr<UClass>>& GetStreamingWhiteListClasses() const { return StreamingWhiteListClasses; }
#endif // WITH_EDITOR
protected:
private:
};