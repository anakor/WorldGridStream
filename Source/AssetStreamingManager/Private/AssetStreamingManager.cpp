
#include "AssetStreamingManager.h"
#include "AssetStreamingCallback.h"
#include "AssetStreamingManagerDebug.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPtr.h"

void UAssetStreamingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    UE_LOG(LogAssetStreamingManager, Log, TEXT("[AssetStreamingEngineSubsystem] Initialized"));

    if (UnloadDelaySeconds < 0.0f)
    {
        UE_LOG(LogAssetStreamingManager, Error, TEXT("UnloadDelaySeconds cannot be a negative number. Setting it to 5 seconds."));
        UnloadDelaySeconds = 5.0f;
    }
}

void UAssetStreamingManager::Deinitialize()
{
    RegisteredAssets.Empty();
    AssetRequestCount.Empty();
    KeepAlive.Empty();
    UnloadTimers.Empty();
}

void UAssetStreamingManager::Tick(float DeltaTime)
{
    TArray<FSoftObjectPath> ToUnload;
    for (auto& Pair : UnloadTimers)
    {
        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.f)
        {
            ToUnload.Add(Pair.Key);
        }
    }

    for (const FSoftObjectPath& Path : ToUnload)
    {
        UnloadTimers.Remove(Path);

        if (AssetRequestCount.Contains(Path))
            continue;

        KeepAlive.Remove(Path);

        TArray<TSharedRef<FStreamableHandle>> Handles;
        if (StreamableManager.GetActiveHandles(Path, Handles, true))
        {
            for (TSharedRef<FStreamableHandle> Handle : Handles)
            {
                Handle->CancelHandle();
            }
        }
    }
}

TStatId UAssetStreamingManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UAssetStreamingManager, STATGROUP_Tickables);
}

bool UAssetStreamingManager::RequestAssetStreaming(
    const TSoftObjectPtr<UObject>& Asset,
    const TScriptInterface<IAssetStreamingCallback>& Callback,
    FGuid& OutRequestId)
{
    OutRequestId = FGuid::NewGuid();
    StreamAsset(Asset, OutRequestId, Callback);

    return OutRequestId.IsValid();
}

bool UAssetStreamingManager::RequestAssetStreamingMultiple(
    const TArray<TSoftObjectPtr<UObject>>& Assets,
    const TScriptInterface<IAssetStreamingCallback>& Callback,
    FGuid& OutRequestId)
{
    if (Assets.Num() == 0)
    {
        OutRequestId.Invalidate();
        return false;
    }

    OutRequestId = FGuid::NewGuid();
    for (TSoftObjectPtr<UObject> Asset : Assets)
    {
        StreamAsset(Asset, OutRequestId, Callback);
    }

    return OutRequestId.IsValid();
}

bool UAssetStreamingManager::ReleaseAsset(FGuid& RequestId)
{
    if (!RequestId.IsValid() || !RegisteredAssets.Contains(RequestId))
    {
        RequestId.Invalidate();
        return false;
    }

    FAssetHandleArray& Assets = RegisteredAssets[RequestId];

    for (int32 i = Assets.Num() - 1; i >= 0; --i)
    {
        const TSoftObjectPtr<UObject>& Asset = Assets[i].Asset;
        const FSoftObjectPath Path = Asset.ToSoftObjectPath();

        if (!AssetRequestCount.Contains(Path)) continue;

        AssetRequestCount[Path]--;

        if (AssetRequestCount[Path] > 0)
        {
            Assets.RemoveAt(i);
        }
        else
        {
            AssetRequestCount.Remove(Path);
        }

        if (!KeepAlive.Contains(Path) || KeepAlive[Path] != Assets[i].Handle)
        {
            if (Assets[i].Handle.IsValid())
            {
                Assets[i].Handle->CancelHandle();
            }
        }
    }

    RegisteredAssets.Remove(RequestId);

    for (const FAssetHandleStruct& AssetHandleStrucdt : Assets)
    {
        const FSoftObjectPath Path = AssetHandleStrucdt.Asset.ToSoftObjectPath();
        if (!UnloadTimers.Contains(Path))
        {
            UnloadTimers.Add(Path, UnloadDelaySeconds);
        }
    }

    RequestId.Invalidate();
    return true;
}

bool UAssetStreamingManager::K2_RequestMultipleAssetStreaming(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, FGuid& OutAssetRequestId)
{
    return RequestAssetStreamingMultiple(AssetsToStream, nullptr, OutAssetRequestId);
}

bool UAssetStreamingManager::K2_RequestMultipleAssetStreamingWithCallback(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId)
{
    return RequestAssetStreamingMultiple(AssetsToStream, AssetLoadedCallback, OutAssetRequestId);
}

bool UAssetStreamingManager::K2_RequestAssetStreaming(const TSoftObjectPtr<UObject>& AssetToStream, FGuid& OutAssetRequestId)
{
    return RequestAssetStreaming(AssetToStream, nullptr, OutAssetRequestId);
}

bool UAssetStreamingManager::K2_RequestAssetStreamingWithCallback(const TSoftObjectPtr<UObject>& AssetToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId)
{
    return RequestAssetStreaming(AssetToStream, AssetLoadedCallback, OutAssetRequestId);
}

bool UAssetStreamingManager::K2_ReleaseAssets(UPARAM(Ref) FGuid& RequestId)
{
    return ReleaseAsset(RequestId);
}

void UAssetStreamingManager::StreamAsset(const TSoftObjectPtr<UObject>& Asset, const FGuid& RequestId, const TScriptInterface<IAssetStreamingCallback>& Callback)
{
    if (Asset.IsNull()) return;

    FStreamableDelegate OnLoaded;
    const bool bIsAssetLoaded = Asset.IsValid();
    OnLoaded.BindLambda([this, Asset, Callback, bIsAssetLoaded]() { HandleAssetLoaded(Asset, Callback, bIsAssetLoaded); });
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(Asset.ToSoftObjectPath(), OnLoaded, FStreamableManager::DefaultAsyncLoadPriority, true);

    if (!RegisteredAssets.Contains(RequestId))
    {
        RegisteredAssets.Add(RequestId, {});
    }
    RegisteredAssets[RequestId].Add(FAssetHandleStruct(Asset, Handle));

    if (!KeepAlive.Contains(Asset.ToSoftObjectPath()))
    {
        KeepAlive.Add(Asset.ToSoftObjectPath(), Handle);
    }

    AssetRequestCount.FindOrAdd(Asset.ToSoftObjectPath())++;
}

void UAssetStreamingManager::HandleAssetLoaded(const TSoftObjectPtr<UObject>& Asset, const TScriptInterface<IAssetStreamingCallback>& Callback, bool bAlreadyLoaded)
{
    if (Asset.IsValid() && Callback.GetObject() && Callback.GetObject()->IsValidLowLevel())
    {
        IAssetStreamingCallback::Execute_OnAssetLoaded(Callback.GetObject(), Asset, bAlreadyLoaded);
    }
}


