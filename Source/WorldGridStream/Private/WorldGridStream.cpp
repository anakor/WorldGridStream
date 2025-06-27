// Copyright Epic Games, Inc. All Rights Reserved.

#include "WorldGridStream.h"
#include "WorldGridStreamPrivate.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "HAL/PlatformProcess.h"
#include "WorldGridStreamLibrary/ExampleLibrary.h"

#include "WorldGridStreamSettings.h"
#include "WorldGridStreamInstancesActor.h"

#define LOCTEXT_NAMESPACE "FWorldGridStreamModule"

DEFINE_LOG_CATEGORY(LogWGS);

void FWorldGridStreamModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	LoadThirdPartyLibrary();
	
	FWorldDelegates::OnPostWorldCreation.AddStatic(
		&WorldCreationEventFunction
	);

	FWorldDelegates::OnPreWorldFinishDestroy.AddStatic(
		&WorldDestroyEventFunction
	);
}

void FWorldGridStreamModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	UnloadThirdPartyLibrary();
}

void FWorldGridStreamModule::LoadThirdPartyLibrary()
{
	if (ThirdPartyLibraryHandle == nullptr)
	{
		// Get the base directory of this plugin
		FString BaseDir = IPluginManager::Get().FindPlugin("WorldGridStream")->GetBaseDir();

		// Add on the relative location of the third party dll and load it
		FString LibraryPath;
	#if PLATFORM_WINDOWS
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/WorldGridStreamLibrary/Win64/ExampleLibrary.dll"));
	#elif PLATFORM_MAC
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/WorldGridStreamLibrary/Mac/Release/libExampleLibrary.dylib"));
	#elif PLATFORM_LINUX
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/WorldGridStreamLibrary/Linux/x86_64-unknown-linux-gnu/libExampleLibrary.so"));
	#endif // PLATFORM_WINDOWS

		ThirdPartyLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

		if (ThirdPartyLibraryHandle)
		{
			// Call the test function in the third party library that opens a message box
			//ThirdPartyLibraryFunction();
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load ThirdParty third party library"));
		}
	}
}

void FWorldGridStreamModule::UnloadThirdPartyLibrary()
{
	if (nullptr != ThirdPartyLibraryHandle)
	{
		FPlatformProcess::FreeDllHandle(ThirdPartyLibraryHandle);
		ThirdPartyLibraryHandle = nullptr;
	}
}
bool FWorldGridStreamModule::IsThirdPartyLibraryLoaded() const
{
	return ThirdPartyLibraryHandle != nullptr;
}
void FWorldGridStreamModule::WorldCreationEventFunction(UWorld* InWorld)
{
	if (nullptr == InWorld)
	{
		return;
	}
	if (false == InWorld->PerModuleDataObjects.FindItemByClass<AWorldGridStreamSettings>())
	{
		return;
	}

	EObjectFlags NewWorldGridInstancesMapFlags = RF_NoFlags;
	if (!InWorld->PerModuleDataObjects.FindItemByClass<AWorldGridStreamInstancesActor>())
	{
		if (InWorld->HasAnyFlags(RF_Transactional))
		{
			NewWorldGridInstancesMapFlags = RF_Transactional;
		}
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags = NewWorldGridInstancesMapFlags;
		AWorldGridStreamInstancesActor* InstancesMap = InWorld->SpawnActor<AWorldGridStreamInstancesActor>(SpawnParameters);
		InstancesMap->SetWorld(InWorld);
		InWorld->PerModuleDataObjects.Add(InstancesMap);
	}
}

void FWorldGridStreamModule::WorldDestroyEventFunction(UWorld* InWorld)
{
	InWorld->PerModuleDataObjects.RemoveAll(
		[](UObject* Object)
		{
			return Object != nullptr && Object->IsA(AWorldGridStreamInstancesActor::StaticClass());
		}
	);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FWorldGridStreamModule, WorldGridStream)
