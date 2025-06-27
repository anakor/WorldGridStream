// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridStreamBuilder.h"
#include "EngineUtils.h"
#include "Math/IntVector.h"
#include "UObject/SavePackage.h"
#include "Algo/ForEach.h"
#include "Algo/Transform.h"
#if WITH_EDITOR
#include "PackageSourceControlHelper.h"
#endif //WITH_EDITOR
#include "WorldPartition/WorldPartition.h"
#include "Landscape.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/StaticMeshActor.h"

#include "WorldGridStreamPrivate.h"
#include "WorldGridStreamMathHelpers.h"
#include "WorldGridStreamConfigs.h"

BEGIN_FUNCTION_BUILD_OPTIMIZATION

#if WITH_EDITOR
bool FWorldGridStreamBuilder::RunBuilder(UWorld* InWorld, int32 InGridSize, bool b2DGrid)
{
	if (!InWorld)
	{
		return false;
	}

	IAssetRegistry& AssetRegistry = FModuleManager::GetModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	AssetRegistry.WaitForCompletion();
	
	bool bResult = true;
	
	FBox EditorBounds;
	UWorldPartition* WorldPartition = InWorld->GetWorldPartition();
	if(nullptr != WorldPartition)
	{
		EditorBounds = WorldPartition->GetEditorWorldBounds();
	}
	else //Landscape Composite World
	{
		for (FActorIterator It(InWorld); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->IsA(ALandscape::StaticClass()))
			{
				ALandscape* Landscape = Cast<ALandscape>(Actor);
				if (Landscape)
				{
					EditorBounds += Landscape->GetComponentsBoundingBox();
				}
			}
		}
	}
	{
		const FInt64Vector MinGridIndex = FWorldGridStreamMathHelpers::GetGridIndex(EditorBounds.Min, InGridSize, false);
		const FInt64Vector MaxGridIndex = FWorldGridStreamMathHelpers::GetGridIndex(EditorBounds.Max, InGridSize, false);
	}

	UE_LOG(LogWGS, Display, TEXT("Iterative Grid Mode"));
	UE_LOG(LogWGS, Display, TEXT("Grid Size:       %d"), InGridSize);
	UE_LOG(LogWGS, Display, TEXT("WorldBounds:     Min %s, Max %s"), *EditorBounds.Min.ToString(), *EditorBounds.Max.ToString());

	TArray<FInt64Vector> ModifiedGridIndices;
	TMap<FInt64Vector, TArray<AActor*>> ActorsInGridIndexMap;
	const UWorldGridStreamConfigs* WorldGridStreamConfigs = GetDefault<UWorldGridStreamConfigs>();
	if( nullptr == WorldGridStreamConfigs)
	{
		return false;
	}
	for (FActorIterator It(InWorld); It; ++It)
	{
		AActor* Actor = *It;
		if (false == ::IsValid(Actor))
		{
			continue;
		}

		bool bCreatedActor = Actor->GetClass()->HasAnyClassFlags(CLASS_Hidden | CLASS_Transient | CLASS_NotPlaceable);
		if(true == bCreatedActor)
		{
			continue;
		}
		if(Actor->IsEditorOnly())
		{
			continue;
		}
		bool bBlacklisted = false;
		const TArray<TObjectPtr<UClass>>& StreamingBlackListClasses = WorldGridStreamConfigs->GetStreamingBlackListClasses();
		for(UClass* BlacklistClass : StreamingBlackListClasses)
		{
			bBlacklisted = BlacklistClass->IsChildOf(Actor->GetClass());
			if(true == bBlacklisted)
			{
				break;
			}
		}
		if(true == bBlacklisted)
		{
			continue;
		}
		bool bWhitelisted = true;
		const TArray<TObjectPtr<UClass>>& StreamingWhiteListClasses = WorldGridStreamConfigs->GetStreamingWhiteListClasses();
		if(StreamingWhiteListClasses.Num() > 0)
		{
			for(UClass* WhiteClass : StreamingWhiteListClasses)
			{
				bWhitelisted = WhiteClass->IsChildOf(Actor->GetClass());
				if(true == bWhitelisted)
				{
					break;
				}
			}
		}
		if (false == bWhitelisted)
		{
			continue;
		}

		const FInt64Vector GridIndex = FWorldGridStreamMathHelpers::GetGridIndex(Actor->GetActorLocation(), InGridSize, false);
		TArray<AActor*>& ActorsInGridIndex = ActorsInGridIndexMap.FindOrAdd(GridIndex);
		if(true == ActorsInGridIndex.Contains(Actor))
		{
			continue;
		}
		UPackage* ActorPackage = Actor->GetPackage();
		UPackage* WorldPackage = InWorld->GetPackage();
		if(GetTransientPackage() == ActorPackage || WorldPackage != ActorPackage)
		{
			continue;
		}
		ActorsInGridIndex.Emplace(Actor);
		ModifiedGridIndices.AddUnique(GridIndex);
	}
	const FString MapName = InWorld->GetMapName();
	for(const FInt64Vector& ModifiedGridIndex : ModifiedGridIndices)
	{
		const TArray<AActor*> ModifiedActors = ActorsInGridIndexMap.FindRef(ModifiedGridIndex);
		const FString InstancesObjectName = FString::Printf(TEXT("%s_%s"), *MapName, *ModifiedGridIndex.ToString());
		const FString PackageName = FString::Printf(TEXT("/Game/WorldGridStream/%s"), *InstancesObjectName);
		for(AActor* ModifiedActor : ModifiedActors)
		{
			//[TODO] WorldGridStreamInstances에 Actor를 넣을 것
			// WorldGridStreamInstances->AddActor(ModifiedActor);

			// WorldGridStreamInstances에 있는 Actor를 Outliner에 등록할 것. 그러기 위해서는 에디터에서 Loading할 때 아래와 같이 코드 작성
			// 잘 될지는 해봐야 암
			// AActor* NewActor = InWorld->SpawnActor<AActor>(AActor::StaticClass(), Location, Rotation);
			// Actor Property 복사 (deep copy, transient 등 제외)
			// for (TFieldIterator<FProperty> PropIt(LoadedActor->GetClass()); PropIt; ++PropIt)
			// {
			// 	FProperty* Property = *PropIt;
			// 	if (!Property->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient))
			// 	{
			// 		Property->CopyCompleteValue_InContainer(NewActor, LoadedActor);
			// 	}
			// }

			// Components 복사
			// TArray<UActorComponent*> SrcComponents;
			// LoadedActor->GetComponents(SrcComponents);
			// for (UActorComponent* SrcComp : SrcComponents)
			// {
			//	if (SrcComp && SrcComp->IsRegistered())
			//	{
			//		UActorComponent* NewComp = NewObject<UActorComponent>(NewActor, SrcComp->GetClass());
			//		if (NewComp)
			//		{
			//			NewComp->RegisterComponent(); // 월드에 등록
			//			NewComp->AttachToComponent(NewActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			//		}
			// 		for (TFieldIterator<FProperty> PropIt(SrcComp->GetClass()); PropIt; ++PropIt)
			// 		{
			// 			FProperty* Property = *PropIt;
			// 			if (!Property->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient))
			// 			{
			// 				Property->CopyCompleteValue_InContainer(SrcComp, LoadedActor);
			// 			}
			// 		}
			// 	}
			
		}
	}

	return true;
}

bool FWorldGridStreamBuilder::SavePackages(const TArray<UPackage*>& Packages, bool bErrorsAsWarnings/* = false */)
{
	if (Packages.IsEmpty())
	{
		return true;
	}
	UE_LOG(LogWGS, Display, TEXT("Saving %d packages..."), Packages.Num());
	FPackageSourceControlHelper PackageHelper;

	const TArray<FString> PackageFilenames = SourceControlHelpers::PackageFilenames(Packages);

	TArray<FString> PackagesToCheckout;
	TArray<FString> PackagesToAdd;
	TArray<FString> PackageNames;

	PackageNames.Reserve(Packages.Num());
	Algo::Transform(Packages, PackageNames, [](UPackage* InPackage) { return InPackage->GetName(); });

	// Display all affected files : that can be useful when analyzing what was saved (or at least what we tried to) by this commandlet : 
	FStringBuilderBase PackageNamesString;
	Algo::ForEach(PackageNames, [&PackageNamesString](const FString& InPackageName) { PackageNamesString.Appendf(TEXT("\t%s\n"), *InPackageName); });
	UE_LOG(LogWGS, Display, TEXT("%s"), PackageNamesString.ToString());

	bool bSuccess = PackageHelper.GetDesiredStatesForModification(PackageNames, PackagesToCheckout, PackagesToAdd, bErrorsAsWarnings);
	if (!bSuccess && !bErrorsAsWarnings)
	{
		return false;
	}
		
	if (PackagesToCheckout.Num())
	{
		if (!PackageHelper.Checkout(PackagesToCheckout, bErrorsAsWarnings))
		{
			bSuccess = false;
			if (!bErrorsAsWarnings)
			{
				return false;
			}
		}
	}

	// Load existing thumbnails to be able to resave them properly
	for (UPackage* PackageToSave : Packages)
	{
		EnsureLoadingComplete(PackageToSave);
	}

	ResetLoaders(TArray<UObject*>(Packages));

	for (int PackageIndex = 0; PackageIndex < Packages.Num(); ++PackageIndex)
	{
		const FString& PackageFilename = PackageFilenames[PackageIndex];
		// Check readonly flag in case some checkouts failed
		if (!IPlatformFile::GetPlatformPhysical().IsReadOnly(*PackageFilename))
		{
			// Save package
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Standalone;
			if (!UPackage::SavePackage(Packages[PackageIndex], nullptr, *PackageFilenames[PackageIndex], SaveArgs))
			{
				bSuccess = false;
				UE_LOG(LogWGS, Error, TEXT("Error saving package %s."), *Packages[PackageIndex]->GetName());
				if(!bErrorsAsWarnings)
				{
					return false;
				}
			}
		}
		else
		{
			check(bErrorsAsWarnings);
		}
	}

	if (PackagesToAdd.Num())
	{
		if (!PackageHelper.AddToSourceControl(PackagesToAdd, bErrorsAsWarnings))
		{
			return false;
		}
	}

	return bSuccess;
}

bool FWorldGridStreamBuilder::DeletePackages(const TArray<UPackage*>& Packages, bool bErrorsAsWarnings/*  = false */)
{
	return true;
}

bool FWorldGridStreamBuilder::DeletePackages(const TArray<FString>& PackageNames, bool bErrorsAsWarnings/*  = false */)
{
	return true;
}

#endif // WITH_EDITOR

END_FUNCTION_BUILD_OPTIMIZATION