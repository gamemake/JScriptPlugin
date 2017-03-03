// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved. 
#include "JScriptEditorPluginPrivatePCH.h"
#include "ReimportJScriptFactory.h"

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
		//JScriptClass->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UReimportJScriptFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UJScriptBlueprint* JScriptClass = Cast<UJScriptBlueprint>(Obj);
	if (JScriptClass && ensure(NewReimportPaths.Num() == 1))
	{
		// JScriptClass->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
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

	const FString ResolvedSourceFilePath = JScriptClass->SourceFilePath;
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
