#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Tickable.h"

#include "AssetStreamingCallback.h"
#include "AssetStreamingHandle.h"
#include "AssetStreamingManager.generated.h"

typedef TArray<FAssetHandleStruct> FAssetHandleArray;
typedef TArray<TSharedRef<FStreamableHandle>> FStreamableHandleArray;

UCLASS()
class UAssetStreamingManager : public UEngineSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
    
//Variable
public:
protected:
    FStreamableManager StreamableManager;

    TMap<FGuid, FAssetHandleArray> RegisteredAssets;
    TMap<FSoftObjectPath, int32> AssetRequestCount;
    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> KeepAlive;
    TMap<FSoftObjectPath, float> UnloadTimers;
private:

//Function
public:
    ASSETSTREAMINGMANAGER_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    ASSETSTREAMINGMANAGER_API virtual void Deinitialize() override;

    // Tickable
    ASSETSTREAMINGMANAGER_API virtual void Tick(float DeltaTime) override;
    ASSETSTREAMINGMANAGER_API virtual TStatId GetStatId() const override;
    ASSETSTREAMINGMANAGER_API virtual bool IsTickable() const override { return true; }

    ASSETSTREAMINGMANAGER_API bool RequestAssetStreaming(const TSoftObjectPtr<UObject>& Asset, const TScriptInterface<IAssetStreamingCallback>& Callback, FGuid& OutRequestId);
    ASSETSTREAMINGMANAGER_API bool RequestAssetsStreaming(const TArray<TSoftObjectPtr<UObject>>& Assets, const TScriptInterface<IAssetStreamingCallback>& Callback, FGuid& OutRequestId);
    ASSETSTREAMINGMANAGER_API bool ReleaseAsset(FGuid& RequestId);

    // Blueprint
protected:
    UFUNCTION(BlueprintCallable, DisplayName = "Request Assets", Category = "Asset Streaming Functions")
    bool K2_RequestAssetsStreaming(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Assets w/Callback", Category = "Asset Streaming Functions")
    bool K2_RequestAssetsStreamingWithCallback(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Asset Streaming", Category = "Asset Streaming Functions")
    bool K2_RequestAssetStreaming(const TSoftObjectPtr<UObject>& AssetToStream, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Asset Streaming w/Callback", Category = "Asset Streaming Functions")
    bool K2_RequestAssetStreamingWithCallback(const TSoftObjectPtr<UObject>& AssetToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Release Assets", Category = "Asset Streaming Functions")
    bool K2_ReleaseAssets(UPARAM(Ref) FGuid& RequestId);

private:
    void StreamAsset(const TSoftObjectPtr<UObject>& Asset, const FGuid& RequestId, const TScriptInterface<IAssetStreamingCallback>& Callback);
    void HandleAssetLoaded(const TSoftObjectPtr<UObject>& Asset, const TScriptInterface<IAssetStreamingCallback>& Callback, bool bAlreadyLoaded);
};