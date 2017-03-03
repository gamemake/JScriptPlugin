#pragma once

#include "Commandlets/Commandlet.h"
#include "JScriptCommandlet.generated.h"

UCLASS()
class UJScriptCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()

	/** Parsed commandline tokens */
	UPROPERTY()
	TArray<FString> CmdLineTokens;

	/** Parsed commandline switches */
	UPROPERTY()
	TArray<FString> CmdLineSwitches;

	virtual int32 Main(const FString& Params) override;
};
