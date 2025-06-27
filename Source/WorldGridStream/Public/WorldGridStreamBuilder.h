// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"

//#include "WorldGridStreamBuilder.generated.h"


//UCLASS(Abstract, Transient, Config=WorldGridStream, MinimalAPI)
struct FWorldGridStreamBuilder
{
	//GENERATED_BODY()

	//Variables
public:
protected:
private:
	// Functions
public:

#if WITH_EDITOR
	/** InGridSize: Size of each grid cell. WorldGridStreamSettings::VisibilityDistance value.
	* b2DGrid: If true, Z coordinate is ignored (2D grid). WorldGridStreamSettings::bIncludeZDistance value.
	* Returns true if the builder was successfully run, false otherwise.
	*/
	WORLDGRIDSTREAM_API bool RunBuilder(UWorld* InWorld, int InGridSize, bool b2DGrid);

	static WORLDGRIDSTREAM_API bool SavePackages(const TArray<UPackage*>& Packages, bool bErrorsAsWarnings = false);
	static WORLDGRIDSTREAM_API bool DeletePackages(const TArray<UPackage*>& Packages, bool bErrorsAsWarnings = false);
	static WORLDGRIDSTREAM_API bool DeletePackages(const TArray<FString>& PackageNames, bool bErrorsAsWarnings = false);
#endif // WITH_EDITOR

protected:
private:
};