// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptBlueprint.h"
#include "JScriptBlueprintGeneratedClass.h"
#include "JScriptBlueprintCompiler.h"
#include "Kismet2NameValidators.h"
#include "KismetReinstanceUtilities.h"
#include "JScriptContext.h"
#include "JScriptContextComponent.h"

///-------------------------------------------------------------

FJScriptBlueprintCompiler::FJScriptBlueprintCompiler(UJScriptBlueprint* SourceSketch, FCompilerResultsLog& InMessageLog, const FKismetCompilerOptions& InCompilerOptions, TArray<UObject*>* InObjLoaded)
	: Super(SourceSketch, InMessageLog, InCompilerOptions, InObjLoaded)
	, NewJScriptBlueprintClass(NULL)
	, ContextProperty(NULL)
{
}

FJScriptBlueprintCompiler::~FJScriptBlueprintCompiler()
{
}

void FJScriptBlueprintCompiler::CleanAndSanitizeClass(UBlueprintGeneratedClass* ClassToClean, UObject*& OldCDO)
{
	Super::CleanAndSanitizeClass(ClassToClean, OldCDO);

	// Make sure our typed pointer is set
	check(ClassToClean == NewClass);	
	NewJScriptBlueprintClass = CastChecked<UJScriptBlueprintGeneratedClass>((UObject*)NewClass);
	ContextProperty = NULL;
}

void FJScriptBlueprintCompiler::CreateClassVariablesFromBlueprint()
{
	Super::CreateClassVariablesFromBlueprint();

	UJScriptBlueprint* JScriptBP = JScriptBlueprint();
	UJScriptBlueprintGeneratedClass* NewScripClass = CastChecked<UJScriptBlueprintGeneratedClass>(NewClass);
	NewScripClass->JScriptProperties.Empty();

	for (auto& Field : JScriptDefinedFields)
	{
		UClass* InnerType = Field.Class;
		if (Field.Class->IsChildOf(UProperty::StaticClass()))
		{
			FString PinCategory;
			if (Field.Class->IsChildOf(UStrProperty::StaticClass()))
			{
				PinCategory = Schema->PC_String;
			}
			else if (Field.Class->IsChildOf(UFloatProperty::StaticClass()))
			{
				PinCategory = Schema->PC_Float;
			}
			else if (Field.Class->IsChildOf(UIntProperty::StaticClass()))
			{
				PinCategory = Schema->PC_Int;
			}
			else if (Field.Class->IsChildOf(UBoolProperty::StaticClass()))
			{
				PinCategory = Schema->PC_Boolean;
			}
			else if (Field.Class->IsChildOf(UObjectProperty::StaticClass()))
			{
				PinCategory = Schema->PC_Object;
				// @todo: some scripting extensions (that are strongly typed) can handle this better
				InnerType = UObject::StaticClass();
			}
			if (!PinCategory.IsEmpty())
			{
				FEdGraphPinType JScriptPinType(PinCategory, TEXT(""), InnerType, false, false);
				UProperty* JScriptProperty = CreateVariable(Field.Name, JScriptPinType);
				if (JScriptProperty != NULL)
				{
					JScriptProperty->SetMetaData(TEXT("Category"), *JScriptBP->GetName());
					JScriptProperty->SetPropertyFlags(CPF_BlueprintVisible | CPF_Edit);
					NewScripClass->JScriptProperties.Add(JScriptProperty);
				}
			}
		}
	}

	CreateJScriptContextProperty();
}

void FJScriptBlueprintCompiler::CreateJScriptContextProperty()
{
	// The only case we don't need a script context is if the script class derives form UJScriptPluginComponent
	UClass* ContextClass = nullptr;
	if (Blueprint->ParentClass->IsChildOf(AActor::StaticClass()))
	{
		ContextClass = UJScriptContextComponent::StaticClass();
	}
	else if (!Blueprint->ParentClass->IsChildOf(UJScriptPluginComponent::StaticClass()))
	{
		ContextClass = UJScriptContext::StaticClass();
	}

	if (ContextClass)
	{
		FEdGraphPinType JScriptContextPinType(Schema->PC_Object, TEXT(""), ContextClass, false, false);
		ContextProperty = CastChecked<UObjectProperty>(CreateVariable(TEXT("Generated_JScriptContext"), JScriptContextPinType));
		ContextProperty->SetPropertyFlags(CPF_ContainsInstancedReference | CPF_InstancedReference);
	}
}

void FJScriptBlueprintCompiler::CreateFunctionList()
{
	Super::CreateFunctionList();

	if (!Blueprint->ParentClass->IsChildOf(UJScriptPluginComponent::StaticClass()))
	{
		for (auto& Field : JScriptDefinedFields)
		{
			if (Field.Class->IsChildOf(UFunction::StaticClass()))
			{
				CreateJScriptDefinedFunction(Field);
			}
		}
	}
}

void FJScriptBlueprintCompiler::CreateJScriptDefinedFunction(FJScriptField& Field)
{
	check(ContextProperty);
	
	UJScriptBlueprint* JScriptBP = JScriptBlueprint();
	const FString FunctionName = Field.Name.ToString();

	// Create Blueprint Graph which consists of 3 nodes: 'Entry', 'Get JScript Context' and 'Call Function'
	// @todo: once we figure out how to get parameter lists for functions we can add suport for that here

	UEdGraph* JScriptFunctionGraph = NewObject<UEdGraph>(JScriptBP, *FString::Printf(TEXT("%s_Graph"), *FunctionName));
	JScriptFunctionGraph->Schema = UEdGraphSchema_K2::StaticClass();
	JScriptFunctionGraph->SetFlags(RF_Transient);
	
	FKismetFunctionContext* FunctionContext = CreateFunctionContext();
	FunctionContext->SourceGraph = JScriptFunctionGraph;
	FunctionContext->bCreateDebugData = false;

	UK2Node_FunctionEntry* EntryNode = SpawnIntermediateNode<UK2Node_FunctionEntry>(NULL, JScriptFunctionGraph);
	EntryNode->CustomGeneratedFunctionName = Field.Name;
	EntryNode->AllocateDefaultPins();

	UK2Node_VariableGet* GetVariableNode = SpawnIntermediateNode<UK2Node_VariableGet>(NULL, JScriptFunctionGraph);
	GetVariableNode->VariableReference.SetSelfMember(ContextProperty->GetFName());
	GetVariableNode->AllocateDefaultPins();

	
	UK2Node_CallFunction* CallFunctionNode = SpawnIntermediateNode<UK2Node_CallFunction>(NULL, JScriptFunctionGraph);
	CallFunctionNode->FunctionReference.SetExternalMember(TEXT("CallJScriptFunction"), ContextProperty->PropertyClass);
	CallFunctionNode->AllocateDefaultPins();
	UEdGraphPin* FunctionNamePin = CallFunctionNode->FindPinChecked(TEXT("FunctionName"));
	FunctionNamePin->DefaultValue = FunctionName;

	// Link nodes together
	UEdGraphPin* ExecPin = Schema->FindExecutionPin(*EntryNode, EGPD_Output);
	UEdGraphPin* GetVariableOutPin = GetVariableNode->FindPinChecked(ContextProperty->GetName());
	UEdGraphPin* CallFunctionPin = Schema->FindExecutionPin(*CallFunctionNode, EGPD_Input);
	UEdGraphPin* FunctionTargetPin = CallFunctionNode->FindPinChecked(TEXT("self"));
	ExecPin->MakeLinkTo(CallFunctionPin);
	GetVariableOutPin->MakeLinkTo(FunctionTargetPin);
}

void FJScriptBlueprintCompiler::FinishCompilingClass(UClass* Class)
{
	UJScriptBlueprint* JScriptBP = JScriptBlueprint();

	UJScriptBlueprintGeneratedClass* JScriptClass = CastChecked<UJScriptBlueprintGeneratedClass>(Class);
	JScriptClass->SourceCode = JScriptBP->SourceCode;

	// Allow Blueprint Components to be used in Blueprints
	if (JScriptBP->ParentClass->IsChildOf(UJScriptPluginComponent::StaticClass()) && Class != JScriptBP->SkeletonGeneratedClass)
	{
		Class->SetMetaData(TEXT("BlueprintSpawnableComponent"), TEXT("true"));
	}

	Super::FinishCompilingClass(Class);

	// Ff context property has been created, create a DSO and set it on the CDO
	if (ContextProperty)
	{
		UObject* CDO = Class->GetDefaultObject();
		UObject* ContextDefaultSubobject = NewObject<UObject>(CDO, ContextProperty->PropertyClass, "JScriptContext", RF_DefaultSubObject | RF_Public);
		ContextProperty->SetObjectPropertyValue(ContextProperty->ContainerPtrToValuePtr<UObject*>(CDO), ContextDefaultSubobject);
	}
}

void FJScriptBlueprintCompiler::Compile()
{
	JScriptBlueprint()->UpdateSourceCodeIfChanged();
	JScriptContext = FJScriptContextBase::CreateContext(JScriptBlueprint()->SourceCode, NULL, NULL);
	bool Result = true;
	if (JScriptContext.IsValid())
	{
		JScriptDefinedFields.Empty();
		JScriptContext->GetJScriptDefinedFields(JScriptDefinedFields);
	}
	ContextProperty = NULL;

	Super::Compile();
}

void FJScriptBlueprintCompiler::EnsureProperGeneratedClass(UClass*& TargetUClass)
{
	if ( TargetUClass && !( (UObject*)TargetUClass )->IsA(UJScriptBlueprintGeneratedClass::StaticClass()) )
	{
		FKismetCompilerUtilities::ConsignToOblivion(TargetUClass, Blueprint->bIsRegeneratingOnLoad);
		TargetUClass = NULL;
	}
}

void FJScriptBlueprintCompiler::SpawnNewClass(const FString& NewClassName)
{
	NewJScriptBlueprintClass = FindObject<UJScriptBlueprintGeneratedClass>(Blueprint->GetOutermost(), *NewClassName);

	if ( NewJScriptBlueprintClass == NULL )
	{
		NewJScriptBlueprintClass = NewObject<UJScriptBlueprintGeneratedClass>(Blueprint->GetOutermost(), FName(*NewClassName), RF_Public | RF_Transactional);
	}
	else
	{
		// Already existed, but wasn't linked in the Blueprint yet due to load ordering issues
		FBlueprintCompileReinstancer::Create(NewJScriptBlueprintClass);
	}
	NewClass = NewJScriptBlueprintClass;
}

bool FJScriptBlueprintCompiler::ValidateGeneratedClass(UBlueprintGeneratedClass* Class)
{
	bool SuperResult = Super::ValidateGeneratedClass(Class);
	bool Result = UJScriptBlueprint::ValidateGeneratedClass(Class);
	return SuperResult && Result;
}
