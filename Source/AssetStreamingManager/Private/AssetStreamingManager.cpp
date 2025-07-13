
#include "AssetStreamingManager.h"
#include "AssetStreamingCallback.h"
#include "AssetStreamingManagerDebug.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPtr.h"

UAssetStreamingManager::UAssetStreamingManager()
{
	UnloadDelaySeconds = 5.0f;
}

UAssetStreamingManager::~UAssetStreamingManager()
{
}

void UAssetStreamingManager::Init(UWorld* InWorld)
{
	World = InWorld;
	bIsActive = true;
}

void UAssetStreamingManager::Shutdown()
{
	bIsActive = false;
	RegisteredAssets.Empty();
	AssetRequestCount.Empty();
	KeepAlive.Empty();
	PendingUnloadArray.Empty();
}

void UAssetStreamingManager::Tick(float DeltaTime)
{
	for (int32 i = PendingUnloadArray.Num() - 1; i >= 0; --i)
	{
		FPendingUnload& Entry = PendingUnloadArray[i];
		Entry.Countdown -= DeltaTime;

		if (Entry.Countdown <= 0.0f)
		{
			for (FAssetHandleStruct& AssetHandleStruct : Entry.Assets)
			{
				FSoftObjectPath Path = AssetHandleStruct.Asset.ToSoftObjectPath();

				if (!AssetRequestCount.Contains(Path))
				{
					KeepAlive.Remove(Path);

					TArray<TSharedRef<FStreamableHandle>> Handles;
					if (StreamableManager.GetActiveHandles(Path, Handles, true))
					{
						for (auto& Handle : Handles)
						{
							Handle->CancelHandle();
						}
					}
				}
			}
			PendingUnloadArray.RemoveAt(i);
		}
	}
}

TStatId UAssetStreamingManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAssetStreamingManager, STATGROUP_Tickables);
}

bool UAssetStreamingManager::RequestAssetStreaming(const TArray<TSoftObjectPtr<UObject>>& AssetsToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId)
{
	// Invalidate the request id if there is nothing to stream.
	if (AssetsToStream.Num() == 0)
	{
		OutAssetRequestId.Invalidate();
		return false;
	}

	// Assign a new guid to the request.
	OutAssetRequestId = FGuid::NewGuid();

	UE_LOG(LogAssetStreamingManager, Verbose, TEXT("Request to stream %s asset(s) received. Request Id: %s"), *FString::FromInt(AssetsToStream.Num()), *OutAssetRequestId.ToString());
	for (const TSoftObjectPtr<UObject> Asset : AssetsToStream)
	{
		StreamAsset(Asset, OutAssetRequestId, AssetLoadedCallback);
	}

	// Any asset streaming operation that passes assertions but still isn't valid will cause the request id to invalidate.
	return OutAssetRequestId.IsValid();
}

bool UAssetStreamingManager::RequestAssetStreaming(const TSoftObjectPtr<UObject>& AssetToStream, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, FGuid& OutAssetRequestId)
{
	// Assign a new guid to the request.
	OutAssetRequestId = FGuid::NewGuid();
	StreamAsset(AssetToStream, OutAssetRequestId, AssetLoadedCallback);

	return OutAssetRequestId.IsValid();
}

bool UAssetStreamingManager::ReleaseAssets(FGuid& RequestId)
{
	if (!RequestId.IsValid())
	{
		UE_LOG(LogAssetStreamingManager, Warning, TEXT("Attempted to release assets using an invalid Guid."));
		return false;
	}

	if (!RegisteredAssets.Contains(RequestId))
	{
		UE_LOG(LogAssetStreamingManager, Warning, TEXT("Attempted to release assets using id '%s' but it leads to no assets."), *RequestId.ToString());
		RequestId.Invalidate();
		return false;
	}

	FAssetHandleArray Assets = RegisteredAssets[RequestId];

	for (int i = Assets.Num() - 1; i >= 0; --i)
	{
		FAssetHandleStruct& AssetHandleStruct = Assets[i];
		FSoftObjectPath Path = AssetHandleStruct.Asset.ToSoftObjectPath();

		if (AssetRequestCount.Contains(Path))
		{
			AssetRequestCount[Path]--;
			if (AssetRequestCount[Path] > 0)
			{
				Assets.RemoveAt(i);
				continue;
			}
			else
			{
				AssetRequestCount.Remove(Path);
			}
		}

		if (KeepAlive.Contains(Path) && AssetHandleStruct.Handle != KeepAlive[Path])
		{
			AssetHandleStruct.Handle->CancelHandle();
		}
	}

	RegisteredAssets.Remove(RequestId);

	if (Assets.Num() > 0)
	{
		PendingUnloadArray.Add({ UnloadDelaySeconds, Assets });
	}

	RequestId.Invalidate();
	return true;
}

void UAssetStreamingManager::StreamAsset(const TSoftObjectPtr<UObject>& AssetToStream, const FGuid& RequestId, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback)
{
	// Request an asynchronous load of the asset, even if the asset is already loaded. We'll keep the handle.
	FStreamableDelegate OnLoaded;
	const bool bIsAssetLoaded = AssetToStream.IsValid();
	OnLoaded.BindLambda([this, AssetToStream, AssetLoadedCallback, bIsAssetLoaded]() { HandleAssetLoaded(AssetToStream, AssetLoadedCallback, bIsAssetLoaded); });
	TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(AssetToStream.ToSoftObjectPath(), OnLoaded, FStreamableManager::DefaultAsyncLoadPriority, true);

	// Register the asset and its handle to the request Id.
	RegisterAssetToId(AssetToStream, Handle, RequestId);

	// We need to keep one handle alive at all times so that we choose when to unload the asset.
	// To do this, we keep the first handle for each asset and never unload it until we really want to release the asset.
	if (!KeepAlive.Contains(AssetToStream.ToSoftObjectPath()))
	{
		KeepAlive.Add(AssetToStream.ToSoftObjectPath(), Handle);
	}

	// Increment the number of references for the asset.
	IncrementAssetReference(AssetToStream);
}

void UAssetStreamingManager::RegisterAssetToId(const TSoftObjectPtr<UObject>& Asset, const TSharedPtr<FStreamableHandle> Handle, const FGuid& Id)
{
	FAssetHandleStruct AssetHandleStruct = FAssetHandleStruct(Asset, Handle);

	if (RegisteredAssets.Contains(Id))
	{
		if (RegisteredAssets[Id].Contains(AssetHandleStruct))
		{
			UE_LOG(LogAssetStreamingManager, Error, TEXT("Attempted to register asset '%s' to Id '%s' but it already exists there."), *Asset.GetAssetName(), *Id.ToString());
			return;
		}

		RegisteredAssets[Id].Add(AssetHandleStruct
		);
	}
	else
	{
		FAssetHandleArray Array;
		Array.Add(AssetHandleStruct);
		RegisteredAssets.Add(Id, Array);
	}
	UE_LOG(LogAssetStreamingManager, Verbose, TEXT("Registered asset '%s' to Id '%s'."), *Asset.GetAssetName(), *Id.ToString());

}

void UAssetStreamingManager::IncrementAssetReference(const TSoftObjectPtr<UObject>& Asset)
{
	checkf(!Asset.IsNull(), TEXT("Cannot increment asset reference of null asset."));
	FSoftObjectPath AssetPath = Asset.ToSoftObjectPath();

	if (AssetRequestCount.Contains(AssetPath))
	{
		AssetRequestCount[AssetPath]++;
	}
	else
	{
		AssetRequestCount.Add(AssetPath, 1);
	}
}

void UAssetStreamingManager::HandleAssetLoaded(const TSoftObjectPtr<UObject>& LoadedAsset, const TScriptInterface<IAssetStreamingCallback>& AssetLoadedCallback, const bool& bAlreadyLoaded)
{
	if (LoadedAsset.IsValid() && AssetLoadedCallback.GetObject()->IsValidLowLevel())
	{
		IAssetStreamingCallback::Execute_OnAssetLoaded(AssetLoadedCallback.GetObject(), LoadedAsset, bAlreadyLoaded);
	}
}

void UAssetStreamingManager::ScheduleAssetUnloading(const FAssetHandleArray& Assets)
{
	if (Assets.Num() == 0)
	{
		UE_LOG(LogAssetStreamingManager, Warning, TEXT("Attempted to schedule asset unloading with an empty array."));
		return;
	}

	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	FTimerHandle Handle;
	FTimerDelegate Delegate;
	Delegate.BindLambda([this, Assets]() { FinalUnloadAssets(Assets); });

	TimerManager.SetTimer(Handle, Delegate, UnloadDelaySeconds, false);
}

void UAssetStreamingManager::FinalUnloadAssets(const FAssetHandleArray& Assets)
{
	// If this somehow runs while the game is quitting, ignore.
	if (!this) return;

	int32 UnloadedAssetsCount = 0;
	for (FAssetHandleStruct AssetHandleStruct : Assets)
	{
		TSoftObjectPtr<UObject> Asset = AssetHandleStruct.Asset;
		checkf(!Asset.IsNull(), TEXT("Attempted to unload null asset pointer."));

		FSoftObjectPath AssetPath = Asset.ToSoftObjectPath();

		// Check if a new request to this asset was made. If so, we won't unload it.
		if (AssetRequestCount.Contains(AssetPath)) continue;

		UE_LOG(LogAssetStreamingManager, VeryVerbose, TEXT("Unloading asset '%s'."), *Asset.GetAssetName());

		// Remove the handle from the KeepAlive array.
		KeepAlive.Remove(AssetPath);

		// Get the active handles for the asset and cancel them. Normally, we should only find one.
		// Cancelling will also stop them from completing if they haven't been loaded yet. The callback won't be called.
		TArray<TSharedRef<FStreamableHandle>> ActiveHandles;
		if (StreamableManager.GetActiveHandles(AssetPath, ActiveHandles, true))
		{
			for (TSharedRef<FStreamableHandle> Handle : ActiveHandles)
			{
				Handle.Get().CancelHandle();
			}

			UnloadedAssetsCount++;
		}
		else
		{
			UE_LOG(LogAssetStreamingManager, Error, TEXT("Attempted to unload asset '%s' but no active handles were found. We should at least find one?"), *Asset.GetAssetName());
		}
	}

	UE_LOG(LogAssetStreamingManager, Verbose, TEXT("Finally unloaded %s assets."), *FString::FromInt(UnloadedAssetsCount));

}
