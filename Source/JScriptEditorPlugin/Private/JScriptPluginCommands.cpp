#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptPluginCommands.h"
#include "JScriptDefinition.h"

FJScriptPluginCommands::FJScriptPluginCommands() : TCommands<FJScriptPluginCommands>(TEXT("JScriptEditorPlugin"), NSLOCTEXT("Contexts", "JScriptEditorPlugin", "JScript"), NAME_None, FEditorStyle::GetStyleSetName())
{
}

PRAGMA_DISABLE_OPTIMIZATION
void FJScriptPluginCommands::RegisterCommands()
{
#define LOCTEXT_NAMESPACE "JScriptEditorPlugin"
	UI_COMMAND(Compile, "Compile", "Full compile scripts", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(Settings, "Settings", "JScriptPlugin settings", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(GenerateDefinition, "Generate Definition", "Generate definition file in scripts directory", EUserInterfaceActionType::Button, FInputGesture());
#undef LOCTEXT_NAMESPACE

	UIActions = MakeShareable(new FUICommandList);
	UIActions->MapAction(
		Compile,
		FExecuteAction::CreateStatic(&FJScriptPluginCommands::CompileAction)
	);
	UIActions->MapAction(
		Settings,
		FExecuteAction::CreateStatic(&FJScriptPluginCommands::SettingsAction)
	);
	UIActions->MapAction(
		GenerateDefinition,
		FExecuteAction::CreateStatic(&FJScriptPluginCommands::GenerateDefinitionAction)
	);
}
PRAGMA_ENABLE_OPTIMIZATION

void FJScriptPluginCommands::CompileAction()
{
	auto Context = JScriptEngine::CreateContext();
	auto JSClass = Context->CompileScript(TEXT("E:\\Unreal Projects\\BotsOfWar\\Intermediate\\Scripts\\a.js"));
	JScriptEngine::FreeContext(Context);
}

void FJScriptPluginCommands::SettingsAction()
{

}

void FJScriptPluginCommands::GenerateDefinitionAction()
{
	FJScriptDefinition Definition;
	Definition.Export(FPaths::Combine(*FPaths::GameDir(), TEXT("Scripts"), TEXT("Definition"), TEXT("ue4.d.ts")));
}

void FJScriptPluginCommands::AddToolbarExtension(FToolBarBuilder &builder)
{
#define LOCTEXT_NAMESPACE "JScriptPlugin"
	builder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateStatic(&FJScriptPluginCommands::GenerateJScriptMenu),
		LOCTEXT("JScriptCombo", "JScript"),
		LOCTEXT("JScriptCombo_Tips", "JScript Plugin"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.ViewOptions", "LevelEditor.ViewOptions.Small"),
		false);
#undef LOCTEXT_NAMESPACE
}

TSharedRef<SWidget> FJScriptPluginCommands::GenerateJScriptMenu()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/ true, FJScriptPluginCommands::Get().UIActions);
#define LOCTEXT_NAMESPACE "JScriptPlugin"
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("JScript", "JScript"));
	MenuBuilder.AddMenuEntry(FJScriptPluginCommands::Get().Compile);
	MenuBuilder.AddMenuEntry(FJScriptPluginCommands::Get().Settings);
	MenuBuilder.AddMenuEntry(FJScriptPluginCommands::Get().GenerateDefinition);
	MenuBuilder.EndSection();
#undef LOCTEXT_NAMESPACE
	return MenuBuilder.MakeWidget();
}
