// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptBlueprintCompiler.h"
#include "KismetCompilerModule.h"
#include "JScriptBlueprintEditor.h"

DEFINE_LOG_CATEGORY(LogJScriptEditorPlugin);

/**
* JScript blueprint editor actions
*/
class FAssetTypeActions_JScriptClass : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions Implementation
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_JScriptClass", "JScript Class"); }
	virtual FColor GetTypeColor() const override { return FColor(201, 29, 85); }
	virtual UClass* GetSupportedClass() const override { return UJScriptBlueprint::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override
	{
		EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;

		for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
		{
			auto Blueprint = Cast<UBlueprint>(*ObjIt);
			if (Blueprint && Blueprint->SkeletonGeneratedClass && Blueprint->GeneratedClass)
			{
				TSharedRef<FJScriptBlueprintEditor> NewBlueprintEditor(new FJScriptBlueprintEditor());

				TArray<UBlueprint*> Blueprints;
				Blueprints.Add(Blueprint);
				NewBlueprintEditor->InitJScriptBlueprintEditor(Mode, EditWithinLevelEditor, Blueprints, true);
			}
			else
			{
				FMessageDialog::Open(EAppMsgType::Ok, 
					FText::FromString(TEXT("JScript Blueprint could not be loaded because it derives from an invalid class. Check to make sure the parent class for this blueprint hasn't been removed!")));
			}
		}
	}
};

/**
* JScript blueprint module
*/
class FJScriptEditorPlugin : public IJScriptEditorPlugin, IBlueprintCompiler
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public:

	virtual bool CanCompile(const UBlueprint* Blueprint) override;

	virtual void PreCompile(UBlueprint* Blueprint) override;
	virtual void Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results, TArray<UObject*>* ObjLoaded) override;
	virtual void PostCompile(UBlueprint* Blueprint) override;
};

IMPLEMENT_MODULE(FJScriptEditorPlugin, JScriptEditorPlugin)

void FJScriptEditorPlugin::StartupModule()
{
	// Register asset types
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(MakeShareable(new FAssetTypeActions_JScriptClass));

	// Register blueprint compiler
	IKismetCompilerInterface& KismetCompilerModule = FModuleManager::LoadModuleChecked<IKismetCompilerInterface>("KismetCompiler");
	KismetCompilerModule.GetCompilers().Add(this);
}

void FJScriptEditorPlugin::ShutdownModule()
{
}

bool FJScriptEditorPlugin::CanCompile(const UBlueprint* Blueprint)
{
	return Cast<UJScriptBlueprint>(Blueprint) != nullptr;
}

void FJScriptEditorPlugin::PreCompile(UBlueprint* Blueprint)
{

}

void FJScriptEditorPlugin::Compile(UBlueprint* Blueprint, const FKismetCompilerOptions& CompileOptions, FCompilerResultsLog& Results, TArray<UObject*>* ObjLoaded)
{
	if ( UJScriptBlueprint* JScriptBlueprint = Cast<UJScriptBlueprint>(Blueprint) )
	{
		FJScriptBlueprintCompiler Compiler(JScriptBlueprint, Results, CompileOptions, ObjLoaded);
		Compiler.Compile();
		check(Compiler.NewClass);
	}
}

void FJScriptEditorPlugin::PostCompile(UBlueprint* Blueprint)
{

}
