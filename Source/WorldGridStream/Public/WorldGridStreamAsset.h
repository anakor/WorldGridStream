// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "WorldGridStreamConvert.generated.h"


class UWorldGridStreamAssets : public UDataAsset
{
	GENERATED_BODY()

// Variables
public:
protected:
private:

//Functions
public:
	void Convert(const FString& InMapName, const FIntPoint& InGridIndex);
protected:
	void ConvertWorldGridToStreamAsset_Impl(const FString& InMapName, const FIntPoint& InGridIndex);
private:
};
