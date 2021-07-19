// Copyright Epic Games, Inc. All Rights Reserved.

#include "Experiment_plugin.h"

#define LOCTEXT_NAMESPACE "FExperiment_pluginModule"

void FExperiment_pluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FExperiment_pluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FExperiment_pluginModule, Experiment_plugin)