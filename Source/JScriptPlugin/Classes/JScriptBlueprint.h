#pragma once

#include "Engine/Blueprint.h"
#include "JScriptBlueprint.generated.h"

/**
* The Script blueprint generates script-defined classes
*/
UCLASS(BlueprintType)
class JSCRIPTPLUGIN_API UJScriptBlueprint : public UBlueprint
{
	GENERATED_UCLASS_BODY()

public:
	/** Source File Path */
	UPROPERTY()
	FString SourceFilePath;

	/** Script source code. @todo: this should be editor-only */
	UPROPERTY()
	FString SourceCode;

#if WITH_EDITOR
	virtual UClass* GetBlueprintClass() const override;
	virtual bool SupportedByDefaultBlueprintFactory() const override;
#endif
};
