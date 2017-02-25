#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptCommandlet.h"

UJScriptCommandlet::UJScriptCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

int32 UJScriptCommandlet::Main(const FString& Params)
{
	bool bSuccess = false;

#if !UE_BUILD_SHIPPING
/*
	const TCHAR* ParamStr = *Params;
	ParseCommandLine(ParamStr, CmdLineTokens, CmdLineSwitches);	

	{
		auto JavascriptIsolate = NewObject<UJavascriptIsolate>();
		auto JavascriptContext = JavascriptIsolate->CreateContext();

		JavascriptContext->Expose(TEXT("Root"), this);
				
		JavascriptContext->AddToRoot();

		JavascriptContext->SetContextId(TEXT("Commandlet"));
		
		{
			FEditorScriptExecutionGuard ScriptGuard;

			if (CmdLineTokens.Num())
			{
				JavascriptContext->RunFile(CmdLineTokens[0]);
			}
		}

		JavascriptContext->JavascriptContext.Reset();

		JavascriptContext->RemoveFromRoot();
	}
*/
#else	
	bSuccess = false;
#endif

	return bSuccess ? 0 : 1;
}