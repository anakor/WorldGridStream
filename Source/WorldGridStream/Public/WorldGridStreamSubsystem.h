// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "WorldGridStreamPrivate.h"
#include "Subsystems/WorldSubsystem.h"

#include "WorldGridStreamSubsystem.generated.h"

UCLASS(MinimalAPI)
class UWorldGridStreamSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
protected:
	UPROPERTY()
	TObjectPtr<class AWorldGridStreamSettings> WorldGridStreamSettings;

	FDelegateHandle ActorSpawnedDelegateHandle;
private:

public:
	UWorldGridStreamSubsystem();
	virtual ~UWorldGridStreamSubsystem();
	
	// Begin UObject overrides
	void PostInitProperties();
	// End UObject overrides

	
	// Begin USubsystem overrides
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem overrides

	// Begin FTickableGameObject overrides
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickableInEditor() const override { return true; }
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual TStatId GetStatId() const override;
	// End FTickableGameObject overrides

#if WITH_EDITOR
	WORLDGRIDSTREAM_API void BuildAll();
#endif //WITH_EDITOR

	class AWorldGridStreamSettings* GetWorldGridStreamSettings() const;
	
protected:
#if WITH_EDITOR
	void OnMapChanged(UWorld* InWorld, EMapChangeType ChangeType);
#endif //WITH_EDITOR
	void OnActorSpawned(AActor* InSpawnedActor);
private:
};
