#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AssetStreamingCallback.generated.h"

UINTERFACE(MinimalAPI)
class UAssetStreamingCallback : public UInterface
{
	GENERATED_BODY()
};

class ASSETSTREAMINGMANAGER_API IAssetStreamingCallback
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, DisplayName = "On Asset Loaded")
	void OnAssetLoaded(const TSoftObjectPtr<UObject>& LoadedAsset, const bool bWasAlreadyLoaded);

	virtual void OnAssetLoaded_Implementation(const TSoftObjectPtr<UObject>& LoadedAsset, const bool bWasAlreadyLoaded);
};