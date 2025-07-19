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

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tickable
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return true; }

    ASSETSTREAMINGMANAGER_API bool RequestAssetStreaming(const TSoftObjectPtr<UObject>& Asset, const TScriptInterface<IAssetStreamingCallback>& Callback, FGuid& OutRequestId);
    ASSETSTREAMINGMANAGER_API bool RequestAssetStreamingMultiple(const TArray<TSoftObjectPtr<UObject>>& Assets, const TScriptInterface<IAssetStreamingCallback>& Callback, FGuid& OutRequestId);
    ASSETSTREAMINGMANAGER_API bool ReleaseAsset(FGuid& RequestId);

    // Blueprint
protected:

    UFUNCTION(BlueprintCallable, DisplayName = "Request Multiple Assets", Category = "Asset Streaming Functions")
    bool K2_RequestMultipleAssetStreaming(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Multiple Assets w/Callback", Category = "Asset Streaming Functions")
    bool K2_RequestMultipleAssetStreamingWithCallback(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Asset Streaming", Category = "Asset Streaming Functions")
    bool K2_RequestAssetStreaming(const TSoftObjectPtr<UObject>& AssetToStream, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Asset Streaming w/Callback", Category = "Asset Streaming Functions")
    bool K2_RequestAssetStreamingWithCallback(const TSoftObjectPtr<UObject>& AssetToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Release Assets", Category = "Asset Streaming Functions")
    bool K2_ReleaseAssets(UPARAM(Ref) FGuid& RequestId);

private:
    void StreamAsset(const TSoftObjectPtr<UObject>& Asset, const FGuid& RequestId, const TScriptInterface<IAssetStreamingCallback>& Callback);
    void HandleAssetLoaded(const TSoftObjectPtr<UObject>& Asset, const TScriptInterface<IAssetStreamingCallback>& Callback, bool bAlreadyLoaded);

private:
    FStreamableManager StreamableManager;
    float UnloadDelaySeconds = 5.0f;

    TMap<FGuid, FAssetHandleArray> RegisteredAssets;
    TMap<FSoftObjectPath, int32> AssetRequestCount;
    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> KeepAlive;
    TMap<FSoftObjectPath, float> UnloadTimers;

};