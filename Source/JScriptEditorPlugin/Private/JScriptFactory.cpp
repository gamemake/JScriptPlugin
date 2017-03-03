// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved. 
#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptFactory.h"

UJScriptFactory::UJScriptFactory(const FObjectInitializer& ObjectInitializer)
	: Super( ObjectInitializer )
{
	SupportedClass = UJScriptBlueprint::StaticClass();
	ParentClass = AActor::StaticClass();

	Formats.Add(TEXT("ts;Typescript"));

	bCreateNew = false;
	bEditorImport = true;
	bText = true;
	bEditAfterNew = true;	
}

bool UJScriptFactory::ConfigureProperties()
{
	// Null the parent class so we can check for selection later
	ParentClass = NULL;

	// Load the class viewer module to display a class picker
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::TreeView;
	Options.bShowObjectRootClass = true;
	Options.bIsBlueprintBaseOnly = true;
	Options.bShowUnloadedBlueprints = true;

	const FText TitleText = NSLOCTEXT("EditorFactories", "CreateJScriptOptions", "Pick Parent Class");
	UClass* ChosenClass = NULL;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UJScriptBlueprintGeneratedClass::StaticClass());
	if (bPressedOk)
	{
		ParentClass = ChosenClass;
	}

	return bPressedOk;
}


bool UJScriptFactory::DoesSupportClass(UClass* Class)
{
	return Class == UJScriptBlueprint::StaticClass();
}

UObject* UJScriptFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	FString GameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	FString TSFilename = FPaths::ConvertRelativePathToFull(Filename);

	if (!TSFilename.StartsWith(GameDir))
	{
		UE_LOG(LogJScriptEditorPlugin, Error, TEXT("Filename must startswith GameDir(%s)."), *GameDir);
		return nullptr;
	}

	GEditor->SelectNone(true, true, false);

	// compile ts file to single js file
	FString JSFilename = TSFilename;

	// read 
	FString SourceCode;
	if (!FFileHelper::LoadFileToString(SourceCode, *JSFilename))
	{
		UE_LOG(LogJScriptEditorPlugin, Error, TEXT("Failed to load file '%s' to string"), *JSFilename);
		return nullptr;
	}

	// verity source code

	UJScriptBlueprint* NewBlueprint = nullptr;
	NewBlueprint = Cast<UJScriptBlueprint>(FindObject<UBlueprint>(InParent, *InName.ToString()));
	if (NewBlueprint != nullptr)
	{
		NewBlueprint->Modify();
	}
	else
	{
		NewBlueprint = CastChecked<UJScriptBlueprint>(FKismetEditorUtilities::CreateBlueprint(ParentClass, InParent, InName, BPTYPE_Normal, UJScriptBlueprint::StaticClass(), UJScriptBlueprintGeneratedClass::StaticClass(), "UJScriptFactory"));
	}

	NewBlueprint->SourceFilePath = TSFilename;
	NewBlueprint->SourceCode = SourceCode;

	// Need to make sure we compile with the new source code
	FKismetEditorUtilities::CompileBlueprint(NewBlueprint);

	FEditorDelegates::OnAssetPostImport.Broadcast(this, NewBlueprint);

	return NewBlueprint;
}
