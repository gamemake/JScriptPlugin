// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "KismetCompiler.h"

/**
* JScript Blueprint Compiler
*/
class FJScriptBlueprintCompiler : public FKismetCompilerContext
{
protected:

	typedef FKismetCompilerContext Super;

public:
	FJScriptBlueprintCompiler(UJScriptBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions, TArray<UObject*>* InObjLoaded);
	virtual ~FJScriptBlueprintCompiler();

	// FKismetCompilerContext
	virtual void Compile() override;
	// End FKismetCompilerContext

protected:
	UJScriptBlueprint* JScriptBlueprint() const { return Cast<UJScriptBlueprint>(Blueprint); }

	// FKismetCompilerContext
	virtual void SpawnNewClass(const FString& NewClassName) override;
	virtual void CleanAndSanitizeClass(UBlueprintGeneratedClass* ClassToClean, UObject*& OldCDO) override;
	virtual void EnsureProperGeneratedClass(UClass*& TargetClass) override;
	virtual void CreateClassVariablesFromBlueprint() override;
	virtual void CreateFunctionList() override;
	virtual void FinishCompilingClass(UClass* Class) override;
	virtual bool ValidateGeneratedClass(UBlueprintGeneratedClass* Class) override;
	// End FKismetCompilerContext

protected:

	/**
	 * Creates a script context property for this class (if needed)
	 */
	void CreateJScriptContextProperty();

	/**
	 * Creates a Blueprint Graph function definition for script defined function
	 *
	 * @param Field Function exported by script
	 */
	void CreateJScriptDefinedFunction(FJScriptField& Field);

	/** New script class */
	UJScriptBlueprintGeneratedClass* NewJScriptBlueprintClass;
	/** JScript context */
	TAutoPtr<FJScriptContextBase> JScriptContext;
	/** JScript-defined properties and functions */
	TArray<FJScriptField> JScriptDefinedFields;
	/** JScript context property generated for the compiled class */
	UObjectProperty* ContextProperty;
};

