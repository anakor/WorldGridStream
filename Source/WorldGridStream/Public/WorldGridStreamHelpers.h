// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FWorldGridStreamHelpers
{

	//Variable declarations
public:	
protected:
private:

	//Function declarations
public:
	static WORLDGRIDSTREAM_API bool HasExceededMaxMemory();
	static WORLDGRIDSTREAM_API bool ShouldCollectGarbage();
	static WORLDGRIDSTREAM_API void DoCollectGarbage();

	// Simulate an engine frame tick
	static WORLDGRIDSTREAM_API void FakeEngineTick(UWorld* World);

protected:

private:
};