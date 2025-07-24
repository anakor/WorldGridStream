#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "Tickable.h"

#include "AssetStreamingCallback.h"
#include "AssetStreamingHandle.h"
#include "AssetStreamingSubsystem.generated.h"

typedef TArray<FAssetHandleStruct> FAssetHandleArray;
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

    TMap<FGuid, FAssetHandleArray> RegisteredAssets;
    TMap<FSoftObjectPath, int32> AssetRequestCount;
    TMap<FSoftObjectPath, TSharedPtr<FStreamableHandle>> KeepAlive;
    TMap<FSoftObjectPath, float> UnloadTimers;
private:

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

    ASSETSTREAMINGMANAGER_API bool RequestAssetStreaming(const FSoftObjectPath& AssetPath, FGuid& OutRequestId);
    ASSETSTREAMINGMANAGER_API bool RequestAssetsStreaming(const TArray<FSoftObjectPath>& AssetPaths, TArray<FGuid>& OutRequestId);
    ASSETSTREAMINGMANAGER_API bool ReleaseAsset(FGuid& RequestId);

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