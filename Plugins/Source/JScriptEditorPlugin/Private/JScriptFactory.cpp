// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved. 
#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptBlueprintGeneratedClass.h"

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

	NewBlueprint->SourceCode = SourceCode;

	NewBlueprint->AssetImportData->Update(CurrentFilename);

	// Need to make sure we compile with the new source code
	FKismetEditorUtilities::CompileBlueprint(NewBlueprint);

	FEditorDelegates::OnAssetPostImport.Broadcast(this, NewBlueprint);

	return NewBlueprint;
}


/** UReimportJScriptFactory */

UReimportJScriptFactory::UReimportJScriptFactory(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

bool UReimportJScriptFactory::ConfigureProperties()
{
	return UFactory::ConfigureProperties();
}

bool UReimportJScriptFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UJScriptBlueprint* JScriptClass = Cast<UJScriptBlueprint>(Obj);
	if (JScriptClass)
	{
		JScriptClass->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UReimportJScriptFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UJScriptBlueprint* JScriptClass = Cast<UJScriptBlueprint>(Obj);
	if (JScriptClass && ensure(NewReimportPaths.Num() == 1))
	{
		JScriptClass->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

/**
* Reimports specified texture from its source material, if the meta-data exists
*/
EReimportResult::Type UReimportJScriptFactory::Reimport(UObject* Obj)
{
	UJScriptBlueprint* JScriptClass = Cast<UJScriptBlueprint>(Obj);
	if (!JScriptClass)
	{
		return EReimportResult::Failed;
	}

	TGuardValue<UJScriptBlueprint*> OriginalJScriptGuardValue(OriginalJScript, JScriptClass);

	const FString ResolvedSourceFilePath = JScriptClass->AssetImportData->GetFirstFilename();
	if (!ResolvedSourceFilePath.Len())
	{
		return EReimportResult::Failed;
	}

	UE_LOG(LogJScriptEditorPlugin, Log, TEXT("Performing atomic reimport of [%s]"), *ResolvedSourceFilePath);

	// Ensure that the file provided by the path exists
	if (IFileManager::Get().FileSize(*ResolvedSourceFilePath) == INDEX_NONE)
	{
		UE_LOG(LogJScriptEditorPlugin, Warning, TEXT("Cannot reimport: source file cannot be found."));
		return EReimportResult::Failed;
	}

	bool OutCanceled = false;

	if (ImportObject(JScriptClass->GetClass(), JScriptClass->GetOuter(), *JScriptClass->GetName(), RF_Public | RF_Standalone, ResolvedSourceFilePath, nullptr, OutCanceled) != nullptr)
	{
		UE_LOG(LogJScriptEditorPlugin, Log, TEXT("Imported successfully"));
		// Try to find the outer package so we can dirty it up
		if (JScriptClass->GetOuter())
		{
			JScriptClass->GetOuter()->MarkPackageDirty();
		}
		else
		{
			JScriptClass->MarkPackageDirty();
		}
	}
	else if(OutCanceled)
	{
		UE_LOG(LogJScriptEditorPlugin, Warning, TEXT("-- import canceled"));
	}
	else
	{
		UE_LOG(LogJScriptEditorPlugin, Warning, TEXT("-- import failed"));
	}

	return EReimportResult::Succeeded;
}

int32 UReimportJScriptFactory::GetPriority() const
{
	return ImportPriority;
}
