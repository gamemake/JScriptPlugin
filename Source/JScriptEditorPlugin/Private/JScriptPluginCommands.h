#pragma once

#include "SlateBasics.h"
#include "Commands.h"
#include "EditorStyle.h"

class FJScriptPluginCommands : public TCommands<FJScriptPluginCommands>
{
public:
	FJScriptPluginCommands();

	virtual void RegisterCommands() override;

	TSharedPtr< FUICommandInfo > Compile;
	TSharedPtr< FUICommandInfo > Settings;
	TSharedPtr< FUICommandInfo > GenerateDefinition;

	TSharedPtr<FUICommandList> UIActions;
	static void CompileAction();
	static void SettingsAction();
	static void GenerateDefinitionAction();
	static void AddToolbarExtension(FToolBarBuilder &builder);
	static TSharedRef<SWidget> GenerateJScriptMenu();
};