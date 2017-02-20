// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

//=============================================================================
// ReimportTextureFactory
//=============================================================================

#pragma once
#include "JScriptFactory.h"
#include "ReimportJScriptFactory.generated.h"

/**
* JScript Blueprint re-import factory
*/
UCLASS(MinimalApi, collapsecategories)
class UReimportJScriptFactory : public UJScriptFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	class UJScriptBlueprint* OriginalJScript;

	virtual bool ConfigureProperties() override;

	//~ Begin FReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;
	//~ End FReimportHandler interface
};



