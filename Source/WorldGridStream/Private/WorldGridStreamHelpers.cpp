// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridStreamHelpers.h"
#include "WorldGridStreamPrivate.h"
#include "Commandlets/Commandlet.h"


bool FWorldGridStreamHelpers::HasExceededMaxMemory()
{
	const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();

	const uint64 MemoryMinFreePhysical = 1llu * 1024 * 1024 * 1024;
	const uint64 MemoryMaxUsedPhysical = FMath::Max(32llu * 1024 * 1024 * 1024, MemStats.TotalPhysical / 2);

	const bool bHasExceededMinFreePhysical = MemStats.AvailablePhysical < MemoryMinFreePhysical;
	const bool bHasExceededMaxUsedPhysical = MemStats.UsedPhysical >= MemoryMaxUsedPhysical;
	const bool bHasExceededMaxMemory = bHasExceededMinFreePhysical || bHasExceededMaxUsedPhysical;

	return bHasExceededMaxMemory;
}

bool FWorldGridStreamHelpers::ShouldCollectGarbage()
{
	return HasExceededMaxMemory();
}

void FWorldGridStreamHelpers::DoCollectGarbage()
{
	const FPlatformMemoryStats MemStatsBefore = FPlatformMemory::GetStats();
	CollectGarbage(IsRunningCommandlet() ? RF_NoFlags : GARBAGE_COLLECTION_KEEPFLAGS, true);
	const FPlatformMemoryStats MemStatsAfter = FPlatformMemory::GetStats();

	UE_LOG(LogWGS, Log, TEXT("GC Performed - Available Physical: %.2fGB, Available Virtual: %.2fGB"),
		(int64)MemStatsAfter.AvailablePhysical / (1024.0 * 1024.0 * 1024.0),
		(int64)MemStatsAfter.AvailableVirtual / (1024.0 * 1024.0 * 1024.0)
	);
}

void FWorldGridStreamHelpers::FakeEngineTick(UWorld* InWorld)
{
	check(InWorld);

	CommandletHelpers::TickEngine(InWorld);
}