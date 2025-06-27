// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FWorldGridStreamModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

protected:
	/** Load the ThirdParty dll and call its entry point */
	void LoadThirdPartyLibrary();
	/** Unload the ThirdParty dll */
	void UnloadThirdPartyLibrary();
	/** Check if the ThirdParty library is loaded */
	bool IsThirdPartyLibraryLoaded() const;

	static void WorldCreationEventFunction(UWorld* InWorld);
	static void WorldDestroyEventFunction(UWorld* InWorld);

private:
	/** Handle to the test dll we will load */
	void*	ThirdPartyLibraryHandle;
};
