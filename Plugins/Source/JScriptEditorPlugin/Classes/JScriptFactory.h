// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "JScriptBlueprint.h"
#include "JScriptFactory.generated.h"

/**
* JScript Blueprint factory
*/
UCLASS(collapsecategories, hidecategories = Object)
class JSCRIPTEDITORPLUGIN_API UJScriptFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY(EditAnywhere, Category = JScriptFactory, meta = (AllowAbstract = "", BlueprintBaseOnly = ""))
	TSubclassOf<class UObject> ParentClass;

	//~ Begin UFactory Interface
	virtual bool ConfigureProperties() override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	//~ End UFactory Interface
};
