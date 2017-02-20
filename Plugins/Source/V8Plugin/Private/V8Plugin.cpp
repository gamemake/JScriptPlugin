#include "V8PluginPCH.h"

DEFINE_LOG_CATEGORY(LogV8Plugin);

class FV8Plugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FV8Plugin, V8Plugin)

void FV8Plugin::StartupModule()
{
}

void FV8Plugin::ShutdownModule()
{
}
