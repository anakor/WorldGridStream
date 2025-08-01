#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Tickable.h"

#include "AssetStreamingCallback.h"
#include "AssetStreamingHandle.h"
#include "AssetStreamingSubsystem.generated.h"

typedef TArray<TSharedRef<FStreamableHandle>> FStreamableHandleArray;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssetLoadedBP, UObject*, LoadedAsset, bool, bAlreadyLoaded);

UCLASS()
class UAssetStreamingSubsystem : public UEngineSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

    //Variable
public:
protected:
    FStreamableManager StreamableManager;

    TMap<FGuid, FAssetHandleStruct> RegisteredAssets;
    TMap<FSoftObjectPath, int32> AssetRequestCount;
    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> KeepAlive;
    TMap<FSoftObjectPath, float> UnloadTimers;


    static constexpr int32 PriorityGroupCount = 11;

    TArray<FAssetRequest> DefaultQueue;
    TArray<FAssetRequest> PriorityQueues[PriorityGroupCount];
    int32 MaxPriorityQueueSize = 8;

public:
    UPROPERTY(BlueprintAssignable, Category = "Asset Streaming Events")
    FOnAssetLoadedBP OnAssetLoaded;

    //Function
public:
    ASSETSTREAMINGMANAGER_API virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    ASSETSTREAMINGMANAGER_API virtual void Deinitialize() override;

    // Tickable
    ASSETSTREAMINGMANAGER_API virtual void Tick(float DeltaTime) override;
    ASSETSTREAMINGMANAGER_API virtual TStatId GetStatId() const override;
    ASSETSTREAMINGMANAGER_API virtual bool IsTickable() const override { return true; }

    ASSETSTREAMINGMANAGER_API virtual int32 GetPriorityGroup(const int32 Priority);

    ASSETSTREAMINGMANAGER_API bool RequestAssetStreaming(const FSoftObjectPath& AssetPath, FGuid& OutRequestId, const int32& Priority = 0);
    ASSETSTREAMINGMANAGER_API bool RequestAssetsStreaming(const TArray<FSoftObjectPath>& AssetPaths, TArray<FGuid>& OutRequestId, const int32& Priority = 0);
	
	ASSETSTREAMINGMANAGER_API UObject* LoadAssetSync(const FSoftObjectPath& AssetPath);

    ASSETSTREAMINGMANAGER_API bool ReleaseAsset(FGuid& RequestId);
	ASSETSTREAMINGMANAGER_API bool ReleaseAssets(const TArray<FGuid>& RequestIds);
    // Blueprint
protected:
    UFUNCTION(BlueprintCallable, DisplayName = "Request Assets", Category = "Asset Streaming Functions")
    bool K2_RequestAssetsStreaming(const TArray<FSoftObjectPath>& AssetsToStream, TArray<FGuid>& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Assets w/Callback", Category = "Asset Streaming Functions")
    bool K2_RequestAssetsStreamingWithCallback(const TArray<FSoftObjectPath>& AssetsToStream, TArray<FGuid>& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Asset Streaming", Category = "Asset Streaming Functions")
    bool K2_RequestAssetStreaming(const FSoftObjectPath& AssetToStream, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Request Asset Streaming w/Callback", Category = "Asset Streaming Functions")
    bool K2_RequestAssetStreamingWithCallback(const FSoftObjectPath& AssetToStream, FGuid& OutAssetRequestId);

    UFUNCTION(BlueprintCallable, DisplayName = "Release Assets", Category = "Asset Streaming Functions")
    bool K2_ReleaseAssets(UPARAM(Ref) FGuid& RequestId);

private:
    void StreamAsset(const FSoftObjectPath& AssetPath, const FGuid& RequestId);
    void HandleAssetLoaded(const FSoftObjectPath& AssetPath, bool bAlreadyLoaded);
};