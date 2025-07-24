
#include "AssetStreamingSubsystem.h"
#include "AssetStreamingCallback.h"
#include "AssetStreamingManagerDebug.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPtr.h"

DECLARE_CYCLE_STAT(TEXT("AssetStreamingManager Tick"), STAT_ASMTick, STATGROUP_AssetStreamingManager);

namespace StreamingManager
{
	float UnloadDelaySeconds = 5.0f;
	FAutoConsoleVariableRef CVarUnloadDelaySeconds(TEXT("StreamingManager.UnloadDelaySeconds")
        , UnloadDelaySeconds
		, TEXT("Seconds to delay before asset unload")
		, ECVF_ReadOnly);

	float LoadDelaySeconds = 0.017f; // 1 / 60 FPS
	FAutoConsoleVariableRef CVarLoadDelaySeconds(TEXT("StreamingManager.LoadDelaySeconds")
        , LoadDelaySeconds
		, TEXT("Seconds to delay before asset load")
		, ECVF_ReadOnly);

    int32 MaxAssetsToLoadPerTick = 20;
	FAutoConsoleVariableRef CVarMaxAssetsToLoadAtOnce(TEXT("StreamingManager.MaxAssetsToLoadPerTick")
        , MaxAssetsToLoadPerTick
		, TEXT("Maximum number of assets that can be loaded at once per tick")
		, ECVF_ReadOnly);
}

void UAssetStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    UE_LOG(LogAssetStreamingManager, Log, TEXT("[UAssetStreamingSystem] Initialized"));
}

void UAssetStreamingSubsystem::Deinitialize()
{
	Super::Deinitialize();

    UE_LOG(LogAssetStreamingManager, Log, TEXT("[UAssetStreamingSystem] Deinitialize"));

    RegisteredAssets.Empty();
    AssetRequestCount.Empty();
    KeepAlive.Empty();
    UnloadTimers.Empty();
}

void UAssetStreamingSubsystem::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_ASMTick);
	TRACE_CPUPROFILER_EVENT_SCOPE(UAssetStreamingSubsystem::Tick);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(AssetStreamingManager);
	LLM_SCOPE_BYNAME(TEXT("AssetStreamingManager"));

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

TStatId UAssetStreamingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UAssetStreamingSubsystem, STATGROUP_Tickables);
}

bool UAssetStreamingSubsystem::RequestAssetStreaming(const TSoftObjectPtr<UObject>& Asset, FGuid& OutRequestId)
{
    OutRequestId = FGuid::NewGuid();
    StreamAsset(Asset, OutRequestId);

    return OutRequestId.IsValid();
}

bool UAssetStreamingSubsystem::RequestAssetsStreaming(const TArray<TSoftObjectPtr<UObject>>& Assets, FGuid& OutRequestId)
{
    if (Assets.Num() == 0)
    {
        OutRequestId.Invalidate();
        return false;
    }

	OutRequestId = FGuid::NewGuid();
    for (TSoftObjectPtr<UObject> Asset : Assets)
    {
        StreamAsset(Asset, OutRequestId);
    }

    return OutRequestId.IsValid();
}

bool UAssetStreamingSubsystem::ReleaseAsset(FGuid& RequestId)
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
            UnloadTimers.Add(Path, StreamingManager::CVarUnloadDelaySeconds->GetFloat());
        }
    }

    RequestId.Invalidate();
    return true;
}

bool UAssetStreamingSubsystem::K2_RequestAssetsStreaming(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, FGuid& OutAssetRequestId)
{
    return RequestAssetsStreaming(AssetsToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_RequestAssetsStreamingWithCallback(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream,  FGuid& OutAssetRequestId)
{
    return RequestAssetsStreaming(AssetsToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_RequestAssetStreaming(const TSoftObjectPtr<UObject>& AssetToStream, FGuid& OutAssetRequestId)
{
    return RequestAssetStreaming(AssetToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_RequestAssetStreamingWithCallback(const TSoftObjectPtr<UObject>& AssetToStream,  FGuid& OutAssetRequestId)
{
    return RequestAssetStreaming(AssetToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_ReleaseAssets(UPARAM(Ref) FGuid& RequestId)
{
    return ReleaseAsset(RequestId);
}

void UAssetStreamingSubsystem::StreamAsset(const TSoftObjectPtr<UObject>& Asset, const FGuid& RequestId)
{
    if (Asset.IsNull()) return;

    const bool bIsAssetLoaded = Asset.IsValid();
    FStreamableDelegate OnLoaded;
    OnLoaded.BindLambda([WeakThis = MakeWeakObjectPtr(this), Asset, bIsAssetLoaded]()
    {
        if (WeakThis.IsValid())
        {
            WeakThis->HandleAssetLoaded(Asset, bIsAssetLoaded);
        }
    });

    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        Asset.ToSoftObjectPath(), OnLoaded, FStreamableManager::DefaultAsyncLoadPriority, true);

    RegisteredAssets.FindOrAdd(RequestId);
    RegisteredAssets[RequestId].Add(FAssetHandleStruct(Asset, Handle));

    if (!KeepAlive.Contains(Asset.ToSoftObjectPath()))
    {
        KeepAlive.Add(Asset.ToSoftObjectPath(), Handle);
    }

    AssetRequestCount.FindOrAdd(Asset.ToSoftObjectPath())++;
}

void UAssetStreamingSubsystem::HandleAssetLoaded(const TSoftObjectPtr<UObject>& Asset, bool bAlreadyLoaded)
{
    if (Asset.IsValid())
    {
        OnAssetLoaded.Broadcast(Asset.Get(), bAlreadyLoaded);
    }
}