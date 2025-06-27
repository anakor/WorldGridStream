// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FWorldGridStreamMathHelpers
{

	//Variable declarations
public:	
protected:
private:

	//Function declarations
public:
	/* * Get the grid index for a given position in the grid.
	 * InGridSize: Size of each grid cell. WorldGridStreamSettings::VisibilityDistance value.
	 * b2DGrid: If true, Z coordinate is ignored (2D grid). WorldGridStreamSettings::bIncludeZDistance value.
	 */
	static FInt64Vector GetGridIndex(const FVector& InPosition, int32 InGridSize, bool b2DGrid);
	
	/* * Get the 2D grid index for a given position in the grid.
	 * InGridSize: Size of each grid cell. WorldGridStreamSettings::VisibilityDistance value.
	 * b2DGrid: If true, Z coordinate is ignored (2D grid). WorldGridStreamSettings::bIncludeZDistance value.
	 */
	static FIntPoint GetGridIndex(const FVector& InPosition, int32 InGridSize);

protected:

private:
};