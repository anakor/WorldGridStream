#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "AssetStreamingHandle.generated.h"


USTRUCT()
struct ASSETSTREAMINGMANAGER_API FAssetHandleStruct
{
	GENERATED_BODY()

public:

	FAssetHandleStruct()
		: Asset(nullptr)
		, Handle(nullptr)
	{}

	FAssetHandleStruct(const FSoftObjectPath& InAsset, const TSharedPtr<FStreamableHandle>& InHandle)
		: Asset(InAsset)
		, Handle(InHandle)
	{}

	FORCEINLINE bool operator==(const FAssetHandleStruct& RHS) const
	{
		return Asset == RHS.Asset;
	}

	FSoftObjectPath Asset;
	TSharedPtr<FStreamableHandle> Handle;
};