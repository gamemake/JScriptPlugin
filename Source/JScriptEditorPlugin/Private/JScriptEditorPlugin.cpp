// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JScriptEditorPluginPrivatePCH.h"
#include "IJScriptEditorPlugin.h"

DEFINE_LOG_CATEGORY(LogJScriptEditorPlugin);

/**
* JScript blueprint module
*/
class FJScriptEditorPlugin : public IJScriptEditorPlugin
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FJScriptEditorPlugin, JScriptEditorPlugin)

void FJScriptEditorPlugin::StartupModule()
{
}

void FJScriptEditorPlugin::ShutdownModule()
{
}
