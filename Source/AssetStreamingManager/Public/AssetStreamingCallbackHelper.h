#pragma once

#include "CoreMinimal.h"
#include "AssetStreamingCallback.h"
#include "AssetStreamingCallbackHelper.generated.h"

UCLASS()
class ASSETSTREAMINGMANAGER_API UAssetStreamingCallbackWrapper : public UObject, public IAssetStreamingCallback
{
    GENERATED_BODY()

public:
    DECLARE_DELEGATE_TwoParams(FOnAssetLoadedDelegate, const TSoftObjectPtr<UObject>&, bool);

    void SetCallback(FOnAssetLoadedDelegate InDelegate);

    virtual void OnAssetLoaded_Implementation(const TSoftObjectPtr<UObject>& Asset, bool bAlreadyLoaded) override;

private:
    FOnAssetLoadedDelegate CallbackDelegate;
};

class FStreamingCallbackHelper
{
public:
    ASSETSTREAMINGMANAGER_API static TScriptInterface<IAssetStreamingCallback> MakeCallback(
        UObject* Outer,
        UAssetStreamingCallbackWrapper::FOnAssetLoadedDelegate Delegate);
};