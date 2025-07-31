#include "AssetStreamingSubsystem.h"
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

	bool bForceLoadComplete = false;
    FAutoConsoleVariable CVarbForceLoadComplete(TEXT("StreamingManager.bForceImmediateLoadComplete")
        , bForceLoadComplete
        , TEXT("Force handle completion for test")
        , ECVF_Default);
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

	int32 AssetsLoaded = 0;

    // PriorityQueue
    for (int32 Bucket = PriorityGroupCount - 1; Bucket >= 0 && AssetsLoaded < StreamingManager::MaxAssetsToLoadPerTick; --Bucket)
    {
        while (PriorityQueues[PriorityGroupCount].Num() > 0 && AssetsLoaded < StreamingManager::MaxAssetsToLoadPerTick)
        {
            FAssetRequest Request = PriorityQueues[Bucket][0];
            PriorityQueues[Bucket].RemoveAt(0);
            StreamAsset(Request.AssetPath, Request.RequestId);
            ++AssetsLoaded;
        }
    }

    // DefaultQueue
    while (DefaultQueue.Num() > 0 && AssetsLoaded < StreamingManager::MaxAssetsToLoadPerTick)
    {
        FAssetRequest Request = DefaultQueue[0];
        DefaultQueue.RemoveAt(0);
        StreamAsset(Request.AssetPath, Request.RequestId);
        ++AssetsLoaded;
    }
}

TStatId UAssetStreamingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UAssetStreamingSubsystem, STATGROUP_Tickables);
}

ASSETSTREAMINGMANAGER_API int32 UAssetStreamingSubsystem::GetPriorityGroup(const int32 Priority)
{
    return FMath::Clamp(Priority / 10, 0, PriorityGroupCount - 1);
}

bool UAssetStreamingSubsystem::RequestAssetStreaming(const FSoftObjectPath& AssetPath, FGuid& OutRequestId, const int32& Priority)
{
    OutRequestId = FGuid::NewGuid();
    FAssetRequest Request(AssetPath, OutRequestId, Priority);

    if (Priority == 0)
    {
        // Default queue
        DefaultQueue.Add(Request);
    }
    else
    {
		int32 PriorityGroup = GetPriorityGroup(Priority);
        // Priority queue
        if (PriorityQueues[PriorityGroup].Num() >= MaxPriorityQueueSize)
        {
            // sort, if queue is full, remove the lowest priority
            PriorityQueues[PriorityGroup].RemoveAt(PriorityQueues[PriorityGroup].Num() - 1);
        }
        PriorityQueues[PriorityGroup].Add(Request);
    }

    return OutRequestId.IsValid();
}

bool UAssetStreamingSubsystem::RequestAssetsStreaming(const TArray<FSoftObjectPath>& AssetPaths, TArray<FGuid>& OutRequestId, const int32& Priority)
{
    if (AssetPaths.Num() == 0)
    {
        OutRequestId.Empty();
        return false;
    }

    for (const FSoftObjectPath& AssetPath : AssetPaths)
    {
		FGuid RequestId = FGuid::NewGuid();
		OutRequestId.Add(RequestId);

		FAssetRequest Request(AssetPath, RequestId, Priority);

        if(Priority == 0)
        {
            // Default queue
            DefaultQueue.Add(Request);
        }
        else
        {
            int32 PriorityGroup = GetPriorityGroup(Priority);
            // Priority queue
            if (PriorityQueues[PriorityGroup].Num() >= MaxPriorityQueueSize)
            {
                // sort, if queue is full, remove the lowest priority
                PriorityQueues[PriorityGroup].RemoveAt(PriorityQueues[PriorityGroup].Num() - 1);
            }
            PriorityQueues[PriorityGroup].Add(Request);
        }
    }

    return true;
}

ASSETSTREAMINGMANAGER_API UObject* UAssetStreamingSubsystem::LoadAssetSync(const FSoftObjectPath& AssetPath)
{
    if (AssetPath.IsNull())
    {
        UE_LOG(LogAssetStreamingManager, Warning, TEXT("LoadAssetSync: AssetPath is null."));
        return nullptr;
    }

    if (IsAsyncLoading())
    {
        UE_LOG(LogAssetStreamingManager, Warning, TEXT("LoadAssetSync: Attempted sync load during async loading phase."));
    }

    UObject* LoadedAsset = AssetPath.TryLoad();

    if (!LoadedAsset)
    {
        UE_LOG(LogAssetStreamingManager, Warning, TEXT("LoadAssetSync: Failed to load asset '%s'."), *AssetPath.ToString());
    }

    return LoadedAsset;
}

bool UAssetStreamingSubsystem::ReleaseAsset(FGuid& RequestId)
{
    if (!RequestId.IsValid() || !RegisteredAssets.Contains(RequestId))
    {
        RequestId.Invalidate();
        return false;
    }
    const FSoftObjectPath Path = RegisteredAssets[RequestId].Asset;
    const TSharedPtr<FStreamableHandle> Handle = RegisteredAssets[RequestId].Handle;

    if (!AssetRequestCount.Contains(Path))
    {
        return false;
    }

    AssetRequestCount[Path]--;
    const bool bKeepAlive = KeepAlive.Contains(Path) && KeepAlive[Path] == Handle;

    if (AssetRequestCount[Path] == 0)
    {
        AssetRequestCount.Remove(Path);
    }

    if (!bKeepAlive && Handle.IsValid())
    {
        Handle->CancelHandle();
    }

    RegisteredAssets.Remove(RequestId);

    if (!UnloadTimers.Contains(Path))
    {
        UnloadTimers.Add(Path, StreamingManager::CVarUnloadDelaySeconds->GetFloat());
    }

    RequestId.Invalidate();
    return true;
}

bool UAssetStreamingSubsystem::ReleaseAssets(const TArray<FGuid>& RequestIds)
{
    bool bAllReleased = true;
    for (FGuid RequestId : RequestIds)
    {
        bAllReleased &= ReleaseAsset(RequestId);
    }
    return bAllReleased;
}

bool UAssetStreamingSubsystem::K2_RequestAssetsStreaming(const TArray<FSoftObjectPath>& AssetsToStream, TArray<FGuid>& OutAssetRequestId)
{
    return RequestAssetsStreaming(AssetsToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_RequestAssetsStreamingWithCallback(const TArray<FSoftObjectPath>& AssetsToStream, TArray<FGuid>& OutAssetRequestId)
{
    return RequestAssetsStreaming(AssetsToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_RequestAssetStreaming(const FSoftObjectPath& AssetToStream, FGuid& OutAssetRequestId)
{
    return RequestAssetStreaming(AssetToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_RequestAssetStreamingWithCallback(const FSoftObjectPath& AssetToStream, FGuid& OutAssetRequestId)
{
    return RequestAssetStreaming(AssetToStream, OutAssetRequestId);
}

bool UAssetStreamingSubsystem::K2_ReleaseAssets(UPARAM(Ref) FGuid& RequestId)
{
    return ReleaseAsset(RequestId);
}

void UAssetStreamingSubsystem::StreamAsset(const FSoftObjectPath& AssetPath, const FGuid& RequestId)
{
    if (AssetPath.IsNull()) return;

    const bool bIsAssetLoaded = StreamableManager.IsAsyncLoadComplete(AssetPath);
    FStreamableDelegate OnLoaded;
    OnLoaded.BindLambda([WeakThis = MakeWeakObjectPtr(this), AssetPath, bIsAssetLoaded]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->HandleAssetLoaded(AssetPath, bIsAssetLoaded);
            }
        });

    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        AssetPath, OnLoaded, FStreamableManager::DefaultAsyncLoadPriority, true);

    RegisteredAssets.FindOrAdd(RequestId);
    RegisteredAssets[RequestId] = FAssetHandleStruct(AssetPath, Handle);

    if (!KeepAlive.Contains(AssetPath))
    {
        KeepAlive.Add(AssetPath, Handle);
    }

    AssetRequestCount.FindOrAdd(AssetPath)++;

#if WITH_EDITOR
    if(StreamingManager::CVarbForceLoadComplete->GetBool())
    {
        HandleAssetLoaded(AssetPath, true);
	}
#endif
}

void UAssetStreamingSubsystem::HandleAssetLoaded(const FSoftObjectPath& AssetPath, bool bAlreadyLoaded)
{
    UObject* LoadedAsset = AssetPath.ResolveObject();
    if (LoadedAsset)
    {
        OnAssetLoaded.Broadcast(LoadedAsset, bAlreadyLoaded);
    }
}