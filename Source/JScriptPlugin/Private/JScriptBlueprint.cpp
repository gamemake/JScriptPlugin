// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "JScriptPluginPrivatePCH.h"
#include "JScriptBlueprint.h"

/////////////////////////////////////////////////////
// UJScriptBlueprint

UJScriptBlueprint::UJScriptBlueprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
UClass* UJScriptBlueprint::GetBlueprintClass() const
{
	return nullptr;// UScriptBlueprintGeneratedClass::StaticClass();
}

bool UJScriptBlueprint::SupportedByDefaultBlueprintFactory() const
{
	return false;
}
#endif