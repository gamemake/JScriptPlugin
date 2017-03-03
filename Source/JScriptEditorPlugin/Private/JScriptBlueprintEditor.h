// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Editor/Kismet/Public/BlueprintEditor.h"

/**
 * JScript blueprint editor (extends Blueprint editor)
 */
class FJScriptBlueprintEditor : public FBlueprintEditor
{
public:
	FJScriptBlueprintEditor();
	virtual ~FJScriptBlueprintEditor();

	/** Initializes script editor */
	void InitJScriptBlueprintEditor(const EToolkitMode::Type Mode, const TSharedPtr< IToolkitHost >& InitToolkitHost, const TArray<UBlueprint*>& InBlueprints, bool bShouldOpenInDefaultsMode);

	/** Work around for the fact there's no other way to prevent Components Mode and Graph Mode from being entered/created */
	virtual void AddApplicationMode(FName ModeName, TSharedRef<FApplicationMode> Mode) override;

	/** @return The JScript blueprint currently being edited in this editor */
	class UJScriptBlueprint* GetJScriptBlueprintObj() const;

	virtual UBlueprint* GetBlueprintObj() const override;
};
