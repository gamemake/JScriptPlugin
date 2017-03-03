// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "JScriptPluginPrivatePCH.h"
#include "JScriptBlueprintGeneratedClass.h"

/////////////////////////////////////////////////////
// UJScriptBlueprintGeneratedClass

UJScriptBlueprintGeneratedClass::UJScriptBlueprintGeneratedClass(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UJScriptBlueprintGeneratedClass::AddUniqueNativeFunction(const FName& InName, Native InPointer)
{
	// Find the function in the class's native function lookup table.
	for (int32 FunctionIndex = 0; FunctionIndex < NativeFunctionLookupTable.Num(); ++FunctionIndex)
	{
		FNativeFunctionLookup& NativeFunctionLookup = NativeFunctionLookupTable[FunctionIndex];
		if (NativeFunctionLookup.Name == InName)
		{
			return;
		}
	}
	new(NativeFunctionLookupTable)FNativeFunctionLookup(InName, InPointer);
}

void UJScriptBlueprintGeneratedClass::RemoveNativeFunction(const FName& InName)
{
	// Find the function in the class's native function lookup table.
	for (int32 FunctionIndex = 0; FunctionIndex < NativeFunctionLookupTable.Num(); ++FunctionIndex)
	{
		FNativeFunctionLookup& NativeFunctionLookup = NativeFunctionLookupTable[FunctionIndex];
		if (NativeFunctionLookup.Name == InName)
		{
			NativeFunctionLookupTable.RemoveAt(FunctionIndex);
			return;
		}
	}
}

void UJScriptBlueprintGeneratedClass::PurgeClass(bool bRecompilingOnLoad)
{
	Super::PurgeClass(bRecompilingOnLoad);
	JScriptProperties.Empty();
}

void FJScriptContextBase::PushJScriptPropertyValues(UJScriptBlueprintGeneratedClass* Class, const UObject* Obj)
{
	// @todo: optimize this
	for (auto Property : Class->JScriptProperties)
	{
		if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
		{
			float Value = FloatProperty->GetFloatingPointPropertyValue(Property->ContainerPtrToValuePtr<float>(Obj));
			SetFloatProperty(Property->GetName(), Value);
		}
		else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
		{
			int32 Value = IntProperty->GetSignedIntPropertyValue(Property->ContainerPtrToValuePtr<int32>(Obj));
			SetIntProperty(Property->GetName(), Value);
		}
		else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
		{
			bool Value = BoolProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<void>(Obj));
			SetBoolProperty(Property->GetName(), Value);
		}
		else if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property))
		{
			UObject* Value = ObjectProperty->GetObjectPropertyValue(Property->ContainerPtrToValuePtr<UObject*>(Obj));
			SetObjectProperty(Property->GetName(), Value);
		}
		else if (UStrProperty* StringProperty = Cast<UStrProperty>(Property))
		{
			FString Value = StringProperty->GetPropertyValue(Property->ContainerPtrToValuePtr<UObject*>(Obj));
			SetStringProperty(Property->GetName(), Value);
		}
	}
}

void FJScriptContextBase::FetchJScriptPropertyValues(UJScriptBlueprintGeneratedClass* Class, UObject* Obj)
{
	// @todo: optimize this
	for (auto Property : Class->JScriptProperties)
	{
		if (UFloatProperty* FloatProperty = Cast<UFloatProperty>(Property))
		{
			float Value = GetFloatProperty(Property->GetName());
			FloatProperty->SetFloatingPointPropertyValue(Property->ContainerPtrToValuePtr<float>(Obj), Value);
		}
		else if (UIntProperty* IntProperty = Cast<UIntProperty>(Property))
		{
			int32 Value = GetIntProperty(Property->GetName());
			IntProperty->SetPropertyValue(Property->ContainerPtrToValuePtr<int32>(Obj), Value);
		}
		else if (UBoolProperty* BoolProperty = Cast<UBoolProperty>(Property))
		{
			bool Value = GetBoolProperty(Property->GetName());
			BoolProperty->SetPropertyValue(Property->ContainerPtrToValuePtr<float>(Obj), Value);
		}
		else if (UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property))
		{
			UObject* Value = GetObjectProperty(Property->GetName());
			ObjectProperty->SetObjectPropertyValue(Property->ContainerPtrToValuePtr<UObject*>(Obj), Value);
		}
		else if (UStrProperty* StringProperty = Cast<UStrProperty>(Property))
		{
			FString Value = GetStringProperty(Property->GetName());
			StringProperty->SetPropertyValue(Property->ContainerPtrToValuePtr<FString>(Obj), Value);
		}
	}
}

FJScriptContextBase* FJScriptContextBase::CreateContext(const FString& SourceCode, UJScriptBlueprintGeneratedClass* Class, UObject* Owner)
{
	FJScriptContextBase* NewContext = NULL;
#if WITH_LUA
	NewContext = new FLuaContext();
#endif
	if (NewContext)
	{
		if (NewContext->Initialize(SourceCode, Owner))
		{
			// Push values set by CDO
			if (Class && Owner)
			{
				NewContext->PushJScriptPropertyValues(Class, Owner);
			}
		}
		else
		{
			delete NewContext;
			NewContext = NULL;
		}
	}
	return NewContext;
}

void FJScriptContextBase::GetSupportedJScriptFileFormats(TArray<FString>& OutFormats)
{
#if WITH_LUA
	OutFormats.Add(TEXT("lua;JScript"));
#endif
}