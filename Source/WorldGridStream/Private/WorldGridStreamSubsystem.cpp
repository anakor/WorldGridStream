// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridStreamSubsystem.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Editor/UnrealEd/Public/Selection.h" // Add this include to resolve the incomplete type error
#endif //WITH_EDITOR
#include "EngineUtils.h"
#if WITH_EDITOR
#include "LevelEditor.h"
#endif //WITH_EDITOR
#include "LandscapeProxy.h"

#include "WorldGridStreamSettings.h"
#include "WorldGridStreamInstancesActor.h"

#define LOCTEXT_NAMESPACE "WorldGridStreamSubsystem"

DECLARE_CYCLE_STAT(TEXT("WorldGridStreamSubsystem Tick"), STAT_WGSSubsystemTick, STATGROUP_WorldGridStream);

BEGIN_FUNCTION_BUILD_OPTIMIZATION

UWorldGridStreamSubsystem::UWorldGridStreamSubsystem()
{
}

UWorldGridStreamSubsystem::~UWorldGridStreamSubsystem()
{
}

void UWorldGridStreamSubsystem::PostInitProperties()
{
	Super::PostInitProperties();
}

void UWorldGridStreamSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    
	const FOnActorSpawned::FDelegate ActorSpawnedDelegate = FOnActorSpawned::FDelegate::CreateUObject(this, &UWorldGridStreamSubsystem::OnActorSpawned);
    if(UWorld* World = GetWorld())
    {
	    ActorSpawnedDelegateHandle = World->AddOnActorSpawnedHandler(ActorSpawnedDelegate);
    }
#if WITH_EDITOR
	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		LevelEditorModule->OnMapChanged().AddUObject(this, &UWorldGridStreamSubsystem::OnMapChanged);
	}
#endif
}

void UWorldGridStreamSubsystem::Deinitialize()
{
	Super::Deinitialize();
    
    if(UWorld* World = GetWorld())
    {
	     World->RemoveOnActorSpawnedHandler(ActorSpawnedDelegateHandle);
    }
#if WITH_EDITOR
	if (FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor"))
	{
		LevelEditorModule->OnMapChanged().RemoveAll(this);
	}
#endif
}

void UWorldGridStreamSubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_WGSSubsystemTick);
	TRACE_CPUPROFILER_EVENT_SCOPE(UWorldGridStreamSubsystem::Tick);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(WGSSubsystem);
	LLM_SCOPE_BYNAME(TEXT("WorldGridStreamSubsystem"));

	Super::Tick(DeltaTime);
}

ETickableTickType UWorldGridStreamSubsystem::GetTickableTickType() const
{
	return HasAnyFlags(RF_ClassDefaultObject) || !GetWorld() || GetWorld()->IsNetMode(NM_DedicatedServer) ? ETickableTickType::Never : ETickableTickType::Always;
}

bool UWorldGridStreamSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return Super::DoesSupportWorldType(WorldType) || WorldType == EWorldType::Inactive;
}

TStatId UWorldGridStreamSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UWorldGridStreamSubsystem, STATGROUP_Tickables);
}

#if WITH_EDITOR
void UWorldGridStreamSubsystem::BuildAll()
{
}
#endif //WITH_EDITOR

AWorldGridStreamSettings* UWorldGridStreamSubsystem::GetWorldGridStreamSettings() const
{
	return WorldGridStreamSettings.Get();
}

#if WITH_EDITOR
void UWorldGridStreamSubsystem::OnMapChanged(UWorld* InWorld, EMapChangeType ChangeType)
{
	if(nullptr == InWorld)
	{
		return;
	}

	if(ChangeType == EMapChangeType::LoadMap)
	{
	}
}
#endif //WITH_EDITOR

void UWorldGridStreamSubsystem::OnActorSpawned(AActor* InSpawnedActor)
{
	if(nullptr == InSpawnedActor)
	{
		return;
	}
	UWorld* World = InSpawnedActor->GetWorld();
	if (true == InSpawnedActor->IsA(AWorldGridStreamSettings::StaticClass()))
	{
		if(nullptr == WorldGridStreamSettings)
		{
			WorldGridStreamSettings = Cast<AWorldGridStreamSettings>(InSpawnedActor);
    		for (TActorIterator<ALandscapeProxy> It(World); It; ++It)
			{
				ALandscapeProxy* Proxy = *It;
				if(nullptr != WorldGridStreamSettings)
				{
					if(ALandscape* Landscape = Proxy->GetLandscapeActor())
					{
						WorldGridStreamSettings->SetWorldScale(Proxy->GetTransform().GetScale3D().X);
						break;
					}
				}
			}
			
			FActorSpawnParameters SpawnParameters;
			EObjectFlags NewWorldGridInstancesActorFlags = RF_NoFlags;
			if (World->HasAnyFlags(RF_Transactional))
			{
				NewWorldGridInstancesActorFlags = RF_Transactional;
			}
			SpawnParameters.ObjectFlags = NewWorldGridInstancesActorFlags;
			AWorldGridStreamInstancesActor* InstanceActor = World->SpawnActor<AWorldGridStreamInstancesActor>(SpawnParameters);
			InstanceActor->SetWorld(World);
			World->PerModuleDataObjects.Emplace(InstanceActor);
		}
		else
		{
			if(WorldGridStreamSettings != InSpawnedActor)
			{
#if WITH_EDITOR
				// 너무많이 호출되어 주석 처리함.
				//FMessageDialog::Open( EAppMsgType::Ok, NSLOCTEXT("ActorStreamingInWorld", "Already Spwaned", "ASW Settings Already Spwaned. Please check Outerline.") );
				if(nullptr != GEditor)
				{
					GEditor->SelectNone(false, false, false);
					GEditor->SelectActor(InSpawnedActor, true, false, true);
					if(nullptr != World)
					{
						GEditor->edactDeleteSelected(World);
					}
				}
				else
				{
					InSpawnedActor->Destroy();
				}
#else
				InSpawnedActor->Destroy();
#endif //WITH_EDITOR
			}
		}
	}
}

END_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
