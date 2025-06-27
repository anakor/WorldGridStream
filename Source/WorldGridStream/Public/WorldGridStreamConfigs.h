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
	TArray<TObjectPtr<UClass>> StreamingBlackListClasses; //Streaming을 하지 않을 Class들을 저장하는 배열.
	
	UPROPERTY(config, EditAnywhere, Category = "World Grid Stream", meta=(DisplayName="Streaming White List Classes"))
	TArray<TObjectPtr<UClass>> StreamingWhiteListClasses; //Streaming을 할 Class들을 저장하는 배열. White List에 있는 Class는 Black List에 있는 Class를 제외하고 Streaming을 한다.
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