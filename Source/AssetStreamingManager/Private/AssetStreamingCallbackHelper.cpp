#include "AssetStreamingCallbackHelper.h"

void UAssetStreamingCallbackWrapper::SetCallback(FOnAssetLoadedDelegate InDelegate)
{
    CallbackDelegate = InDelegate;
}

void UAssetStreamingCallbackWrapper::OnAssetLoaded_Implementation(const TSoftObjectPtr<UObject>& Asset, bool bAlreadyLoaded)
{
    if (CallbackDelegate.IsBound())
    {
        CallbackDelegate.Execute(Asset, bAlreadyLoaded);
    }
}

TScriptInterface<IAssetStreamingCallback> FStreamingCallbackHelper::MakeCallback(UObject* Outer, UAssetStreamingCallbackWrapper::FOnAssetLoadedDelegate Delegate)
{
    UAssetStreamingCallbackWrapper* CallbackObj = NewObject<UAssetStreamingCallbackWrapper>(Outer ? Outer : GetTransientPackage());
    CallbackObj->AddToRoot(); // GC ¹æÁö
    CallbackObj->SetCallback(Delegate);

    TScriptInterface<IAssetStreamingCallback> Interface;
    Interface.SetObject(CallbackObj);
    Interface.SetInterface(Cast<IAssetStreamingCallback>(CallbackObj));
    return Interface;
}
