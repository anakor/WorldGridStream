#pragma once
#include "CoreMinimal.h"
#include "AssetStreamingSubsystem.h"
#include "AssetStreamingTest.generated.h"

UCLASS()
class UAssetStreamingTestHelper : public UObject
{
    GENERATED_BODY()
public:
    bool bDelegateCalled = false;

    UFUNCTION()
    void OnAssetLoaded(UObject* LoadedAsset, bool bAlreadyLoaded)
    {
        bDelegateCalled = true;
    }
};
