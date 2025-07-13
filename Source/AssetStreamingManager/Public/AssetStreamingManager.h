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
class UAssetStreamingManager : public UObject, public FTickableGameObject
{
    GENERATED_BODY()
public:
    UAssetStreamingManager();
    virtual ~UAssetStreamingManager();

    void Init(UWorld* InWorld);
    void Shutdown();

    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return bIsActive; }


public:
    /**
     * Request streaming of multiple assets.
     * Each asset will be streamed one by one.
     * @param AssetsToStream The assets to asynchronously stream.
     * @param AssetLoadedCallback The callback to call when an asset is loaded. Will be called once by asset loaded.
     * @param OutAssetRequestId The request id assigned for your request. Use it to release the assets you streamed. Invalidated if the request was invalid.
     * @returns True if the request was successful.
     */
    bool RequestAssetStreaming(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId);

    /**
     * Request streaming of a single asset.
     * @param AssetToStream The asset to asynchronously stream.
     * @param AssetLoadedCallback The callback to call when an asset is loaded.
     * @param OutAssetRequestId The request id assigned for your request. Use it to release the asset you streamed. Invalidated if the request was invalid.
     * @returns True if the request was successful.
     */
    bool RequestAssetStreaming(const TSoftObjectPtr<UObject>& AssetToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId);

    /**
     * Release the asset you streamed.
     * Warning: must be called when you don't need the streamed assets anymore!
     * @param RequestId The id returned by the streaming request. Will be invalidated after releasing assets.
     * @returns True if releasing was successful.
     */
    bool ReleaseAssets(FGuid& RequestId);

 private:
     void StreamAsset(const TSoftObjectPtr<UObject>& AssetToStream, const FGuid& RequestId, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback);

     void RegisterAssetToId(const TSoftObjectPtr<UObject>& Asset, const TSharedPtr<FStreamableHandle> Handle, const FGuid& Id);

     void IncrementAssetReference(const TSoftObjectPtr<UObject>& Asset);

     void HandleAssetLoaded(const TSoftObjectPtr<UObject>& LoadedAsset, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, const bool& bAlreadyLoaded);

     void ScheduleAssetUnloading(const FAssetHandleArray& Assets);

     void FinalUnloadAssets(const FAssetHandleArray& Assets);

private:
    bool bIsActive;
    TWeakObjectPtr<UWorld> World;

    FStreamableManager StreamableManager;
    
    // The amount of time to wait before finally unloading an asset when its references drop to zero.
    float UnloadDelaySeconds;

    // Maps request guid to the requested assets and their handle.
    TMap<FGuid, FAssetHandleArray> RegisteredAssets;

    // Maps asset paths to the number of requests they have.
    TMap<FSoftObjectPath, int32> AssetRequestCount;

    // Handles to keep alive until we finally unload the asset.
    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> KeepAlive;

    struct FPendingUnload
    {
        float Countdown;
        FAssetHandleArray Assets;
    };

    TArray<FPendingUnload> PendingUnloadArray;

};