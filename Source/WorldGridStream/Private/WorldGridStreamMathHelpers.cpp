// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridStreamMathHelpers.h"

FInt64Vector FWorldGridStreamMathHelpers::GetGridIndex(const FVector& InPosition, int32 InGridSize, bool b2DGrid)
{
	check(InGridSize > 0);

	FVector GridIndex = InPosition / InGridSize;

	// In case of 2D grid, Z coordinate is always 0
	return FInt64Vector(
		FMath::FloorToInt(GridIndex.X),
		FMath::FloorToInt(GridIndex.Y),
		b2DGrid ? 0 : FMath::FloorToInt(GridIndex.Z)
	);
}

FIntPoint FWorldGridStreamMathHelpers::GetGridIndex(const FVector& InPosition, int32 InGridSize)
{
	check(InGridSize > 0);

	FVector GridIndex = InPosition / InGridSize;

	// In case of 2D grid, Z coordinate is always 0
	return FIntPoint(FMath::FloorToInt(GridIndex.X), FMath::FloorToInt(GridIndex.Y));
}