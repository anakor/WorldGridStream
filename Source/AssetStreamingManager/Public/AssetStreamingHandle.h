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

struct FAssetRequest
{
	FSoftObjectPath AssetPath;
	FGuid RequestId;
	int32 Priority; // 0: default queue, 0 ÃÊ°ú: priority queue

	FAssetRequest(const FSoftObjectPath& InPath, const FGuid& InId, int32 InPriority = -1)
		: AssetPath(InPath), RequestId(InId), Priority(InPriority) {
	}
};