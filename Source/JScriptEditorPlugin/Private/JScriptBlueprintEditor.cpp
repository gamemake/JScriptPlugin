// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JScriptEditorPluginPrivatePCH.h"
#include "SKismetInspector.h"
#include "JScriptBlueprintEditor.h"
#include "BlueprintEditorModes.h"

#define LOCTEXT_NAMESPACE "JScriptEditor"

FJScriptBlueprintEditor::FJScriptBlueprintEditor()
{
}

FJScriptBlueprintEditor::~FJScriptBlueprintEditor()
{
	UJScriptBlueprint* Blueprint = GetJScriptBlueprintObj();
	if ( Blueprint )
	{
		Blueprint->OnChanged().RemoveAll(this);
	}
}

void FJScriptBlueprintEditor::InitJScriptBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode)
{
	InitBlueprintEditor(Mode, InitToolkitHost, InBlueprints, bShouldOpenInDefaultsMode);
	SetCurrentMode(FBlueprintEditorApplicationModes::BlueprintDefaultsMode);
}

void FJScriptBlueprintEditor::AddApplicationMode(FName ModeName, TSharedRef<FApplicationMode> Mode)
{
	if (ModeName == FBlueprintEditorApplicationModes::BlueprintDefaultsMode)
	{
		FBlueprintEditor::AddApplicationMode(ModeName, Mode);
	}
}

UJScriptBlueprint* FJScriptBlueprintEditor::GetJScriptBlueprintObj() const
{
	return Cast<UJScriptBlueprint>(GetBlueprintObj());
}

UBlueprint* FJScriptBlueprintEditor::GetBlueprintObj() const
{
	auto Blueprint = FBlueprintEditor::GetBlueprintObj();
	//auto JScriptBlueprint = Cast<UJScriptBlueprint>(Blueprint);
	//JScriptBlueprint->
	return Blueprint;
}

#undef LOCTEXT_NAMESPACE
