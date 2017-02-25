// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "JScriptPluginComponent.generated.h"

/** 
 * JScript-extendable component class
 */
UCLASS(Blueprintable, hidecategories = (Object, ActorComponent), editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup = JScript)
class JSCRIPTPLUGIN_API UJScriptPluginComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category = JScript)
	FString SourceFilename;

	/**
	* Calls a script-defined function (no arguments)
	* @param FunctionName Name of the function to call
	*/
	UFUNCTION(BlueprintCallable, Category = "JScript|Functions")
	virtual bool Execute(FString Code);
	UFUNCTION(BlueprintCallable, Category = "JScript|Functions")
	virtual void Output(FString Message);

	//~ Begin UActorComponent Interface.
	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void OnUnregister() override;
	//~ Begin UActorComponent Interface.

protected:

	/** JScript context (code) */
	IJScriptContext* Context;
};

