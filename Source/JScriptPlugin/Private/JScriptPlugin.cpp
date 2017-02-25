// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JScriptPluginPrivatePCH.h"

DEFINE_LOG_CATEGORY(LogJScriptPlugin);

class FJScriptPlugin : public IJScriptPlugin
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FJScriptPlugin, JScriptPlugin)

void FJScriptPlugin::StartupModule()
{
	JScriptEngine::Initialize();
}

void FJScriptPlugin::ShutdownModule()
{
	JScriptEngine::Finalize();
}
