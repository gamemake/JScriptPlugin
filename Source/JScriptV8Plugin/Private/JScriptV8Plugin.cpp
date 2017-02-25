#include "JScriptV8PluginPCH.h"

DEFINE_LOG_CATEGORY(LogJScriptV8Plugin);

class FJScriptV8Plugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FJScriptV8Plugin, JScriptV8Plugin)

void FJScriptV8Plugin::StartupModule()
{
}

void FJScriptV8Plugin::ShutdownModule()
{
}
