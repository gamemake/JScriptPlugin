#include "JScriptEditorPluginPrivatePCH.h"
#include "JScriptDefinition.h"

#define O(...) Out(FString::Printf(##__VA_ARGS__))

FJScriptDefinition::FJScriptDefinition()
{
}

FJScriptDefinition::~FJScriptDefinition()
{
}

FString FJScriptDefinition::GetClassName(UClass* Class)
{
	if (Class->GetPrefixCPP())
	{
		return FString::Printf(TEXT("%s%s"), Class->GetPrefixCPP(), *Class->GetName());
	}
	else
	{
		return Class->GetName();
	}
}

FString FJScriptDefinition::GetStructName(UScriptStruct* Struct)
{
	if (Struct->GetPrefixCPP())
	{
		return FString::Printf(TEXT("%s%s"), Struct->GetPrefixCPP(), *Struct->GetName());
	}
	else
	{
		return Struct->GetName();
	}
}

FString FJScriptDefinition::GetEnumName(UEnum* Enum)
{
	return Enum->GetName();
}

bool FJScriptDefinition::Export(const FString& ExportToPath)
{
	OutText = TEXT("");
	Segments.Empty();

	BeginSegment();
	O(TEXT("/// <reference path=\"./global.d.ts\" />"));
	EndSegment();

	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->HasAnyClassFlags(CLASS_Deprecated)) continue;
		if (It->HasAnyClassFlags(CLASS_NewerVersionExists)) continue;
		if (It->HasAnyClassFlags(CLASS_CompiledFromBlueprint)) continue;
		if (It->HasAnyClassFlags(RF_Transient)) continue;
		if (FKismetEditorUtilities::IsClassABlueprintSkeleton(*It)) continue;
		FString ClassName = GetClassName(*It);
		if (ClassName.Find("UTRASHCLASS_") == 0) continue;
		if (ClassName.Find("UPLACEHOLDER-") == 0) continue;
		if (ClassName.Find("UORPHANED_DATA_ONLY_") == 0) continue;
		if (!ExportClass(*It, false)) return false;
	}

	FString OriginalText;
	if (FFileHelper::LoadFileToString(OriginalText, *ExportToPath))
	{
		if (OutText == OriginalText)
		{
			return true;
		}
	}
	FFileHelper::SaveStringToFile(OutText, *ExportToPath);

	return true;
}

void FJScriptDefinition::BeginSegment()
{
	FSegment Segment;
	Segments.Push(Segment);
}

void FJScriptDefinition::Out(const FString& line)
{
	auto& Segment = Segments[Segments.Num() - 1];
	Segment.Push(line);
}

void FJScriptDefinition::EndSegment()
{
	auto Segment = Segments.Pop();
	FString Text;
	for (auto& Line : Segment)
	{
		Text += Line;
		Text += TEXT("\n");
	}
	OutText += Text;
	OutText += TEXT("\n");
}

FString FJScriptDefinition::GetTypeName(UProperty* Property)
{
	if (auto Int8Property = Cast<UInt8Property>(Property))
	{
		return TEXT("number /*Int8*/");
	}
	else if (auto Int16Property = Cast<UInt16Property>(Property))
	{
		return TEXT("number /*Int16*/");
	}
	else if (auto IntProperty = Cast<UIntProperty>(Property))
	{
		return TEXT("number /*Int32*/");
	}
	else if (auto Int64Property = Cast<UInt64Property>(Property))
	{
		return TEXT("number /*Int64*/");
	}
	else if (auto ByteProperty = Cast<UByteProperty>(Property))
	{
		if (ByteProperty->Enum)
		{
			ExportEnum(ByteProperty->Enum);
			return GetEnumName(ByteProperty->Enum);
		}
		else
		{
			return TEXT("number /*UInt8*/");
		}
	}
	else if (auto UInt16Property = Cast<UUInt16Property>(Property))
	{
		return TEXT("number /*UInt16*/");
	}
	else if (auto UInt32Property = Cast<UUInt32Property>(Property))
	{
		return TEXT("number /*UInt32*/");
	}
	else if (auto UInt64Property = Cast<UUInt64Property>(Property))
	{
		return TEXT("number /*UInt64*/");
	}
	else if (auto FloatProperty = Cast<UFloatProperty>(Property))
	{
		return TEXT("number /*Float*/");
	}
	else if (auto DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		return TEXT("number /*Double*/");
	}
	else if (auto BoolProperty = Cast<UBoolProperty>(Property))
	{
		return TEXT("boolean");
	}
	else if (auto NameProperty = Cast<UNameProperty>(Property))
	{
		return TEXT("string");
	}
	else if (auto StrProperty = Cast<UStrProperty>(Property))
	{
		return TEXT("string");
	}
	else if (auto TextProperty = Cast<UTextProperty>(Property))
	{
		return TEXT("string");
	}
	else if (auto ClassProperty = Cast<UClassProperty>(Property))
	{
		return TEXT("UClass");
	}
	else if (auto StructProperty = Cast<UStructProperty>(Property))
	{
		auto ScriptStruct = Cast<UScriptStruct>(StructProperty->Struct);
		ExportStruct(ScriptStruct);
		return GetStructName(ScriptStruct);
	}
	else if (auto ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		auto Class = ObjectProperty->PropertyClass;
		ExportClass(Class, false);
		return GetClassName(Class);
	}
	else if (auto InterfaceProperty = Cast<UInterfaceProperty>(Property))
	{
		auto Class = InterfaceProperty->InterfaceClass;
		ExportClass(Class, true);
		return GetClassName(Class);
	}
	else if (auto SetProperty = Cast<USetProperty>(Property))
	{
		auto Inner = SetProperty->ElementProp;
		return FString::Printf(TEXT("TSet<%s>"), *GetTypeName(Inner));
	}
	else if (auto MapProperty = Cast<UMapProperty>(Property))
	{
		auto KeyProp = GetTypeName(MapProperty->KeyProp);
		auto ValueProp = GetTypeName(MapProperty->ValueProp);
		return FString::Printf(TEXT("TMap<%s, %s>"), *KeyProp, *ValueProp);
	}
	else if (auto ArrayProperty = Cast<UArrayProperty>(Property))
	{
		auto Inner = ArrayProperty->Inner;
		return FString::Printf(TEXT("TArray<%s>"), *GetTypeName(Inner));
	}
	else if (auto DelegateProperty = Cast<UDelegateProperty>(Property))
	{
		FString ReturnType;
		FString ParamTypes = GetFuncParamDecls(DelegateProperty->SignatureFunction, ReturnType, true);
		if (ReturnType.Len() > 0)
		{
			if (ParamTypes.Len() > 0)
			{
				return FString::Printf(TEXT("TDelegateRetVal%d<%s, %s>"), DelegateProperty->SignatureFunction->NumParms - 1, *ReturnType, *ParamTypes);
			}
			else
			{
				return FString::Printf(TEXT("TDelegateRetVal<%s>"), *ReturnType);
			}
		}
		else
		{
			if (ParamTypes.Len() > 0)
			{
				return FString::Printf(TEXT("TDelegate%d<%s>"), DelegateProperty->SignatureFunction->NumParms, *ParamTypes);
			}
			else
			{
				return FString::Printf(TEXT("TDelegate"));
			}
		}
	}
	else if (auto MulticastDelegateProperty = Cast<UMulticastDelegateProperty>(Property))
	{
		FString ReturnType;
		FString ParamTypes = GetFuncParamDecls(MulticastDelegateProperty->SignatureFunction, ReturnType, true);
		if (ReturnType.Len() > 0)
		{
			if (ParamTypes.Len() > 0)
			{
				return FString::Printf(TEXT("TMulticastDelegateRetVal%d<%s, %s>"), MulticastDelegateProperty->SignatureFunction->NumParms - 1, *ReturnType, *ParamTypes);
			}
			else
			{
				return FString::Printf(TEXT("TMulticastDelegateRetVal<%s>"), *ReturnType);
			}
		}
		else
		{
			if (ParamTypes.Len() > 0)
			{
				return FString::Printf(TEXT("TMulticastDelegate%d<%s>"), MulticastDelegateProperty->SignatureFunction->NumParms, *ParamTypes);
			}
			else
			{
				return FString::Printf(TEXT("TMulticastDelegate"));
			}
		}
	}

	return TEXT("UNKNOWN_TYPE");
}

FString FJScriptDefinition::GetFuncParamDecls(UFunction* Function, FString& ReturnType, bool NoNames)
{
	FString ParamDecls;
	for (TFieldIterator<UProperty> It(Function); It; ++It)
	{
		if ((It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) != CPF_Parm)
		{
			ReturnType = GetTypeName(*It);
			continue;
		}

		if (!ParamDecls.IsEmpty())
		{
			ParamDecls += TEXT(", ");
		}

		if ((It->PropertyFlags & CPF_ConstParm) || (It->PropertyFlags &CPF_OutParm))
		{
			FString Flags = TEXT("");
			if (It->PropertyFlags & CPF_ConstParm)
			{
				Flags += "CONST";
			}
			if (It->PropertyFlags &CPF_OutParm)
			{
				if (!Flags.IsEmpty()) Flags += TEXT("|");
				Flags += TEXT("OUT");
			}
			ParamDecls += FString::Printf(TEXT("/*%s*/"), *Flags);
		}
		if (NoNames)
		{
			ParamDecls += GetTypeName(*It);
		}
		else
		{
			ParamDecls += FString::Printf(TEXT("%s: %s"), *It->GetName(), *GetTypeName(*It));
		}
	}
	return ParamDecls;
}

bool FJScriptDefinition::ExportStruct(UScriptStruct* Struct)
{
	if (StructSet.Find(Struct->GetFName())) return true;
	StructSet.Add(Struct->GetFName());

	auto SuperStruct = Cast<UScriptStruct>(Struct->GetSuperStruct());
	BeginSegment();

	O(TEXT("// UScriptStruct"));
	if (SuperStruct)
	{
		ExportStruct(SuperStruct);
		O(TEXT("declare interface %s extends %s {"), *GetStructName(Struct), *GetStructName(SuperStruct));
	}
	else
	{
		O(TEXT("declare interface %s {"), *GetStructName(Struct));
	}

	for (TFieldIterator<UProperty> It(Struct, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		O(TEXT("	%s: %s;"), *It->GetName(), *GetTypeName(*It));
	}

	O(TEXT("}"));

	EndSegment();
	return true;
}

bool FJScriptDefinition::ExportClass(UClass* Class, bool IsInterface)
{
	if (ClassSet.Find(Class->GetFName())) return true;
	ClassSet.Add(Class->GetFName());

	BeginSegment();
	
	auto SuperClass = Class->GetSuperClass();
	if(IsInterface) O(TEXT("// UInterface"));
	if (SuperClass)
	{
		ExportClass(SuperClass, IsInterface);
		O(TEXT("declare %s %s extends %s {"), IsInterface? TEXT("interface"): TEXT("class"),  *GetClassName(Class), *GetClassName(SuperClass));
	}
	else
	{
		O(TEXT("declare %s %s {"), IsInterface ? TEXT("interface") : TEXT("class"), *GetClassName(Class));
	}

	for (TFieldIterator<UProperty> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		O(TEXT("	%s: %s;"), *It->GetName(), *GetTypeName(*It));
	}

	for (TFieldIterator<UFunction> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		if (It->FunctionFlags & FUNC_BlueprintCallable)
		{
			FString ReturnType;
			FString ParamDecls = GetFuncParamDecls(*It, ReturnType, false);
			if (ReturnType.Len() == 0) ReturnType = TEXT("void");

			if ((It->FunctionFlags & FUNC_Static))
			{
				O(TEXT("	static %s(%s): %s;"), *It->GetName(), *ParamDecls, *ReturnType);
			}
			else
			{
				O(TEXT("	%s(%s): %s;"), *It->GetName(), *ParamDecls, *ReturnType);
			}
		}
	}

	O(TEXT("}"));

	EndSegment();
	return true;
}

bool FJScriptDefinition::ExportEnum(UEnum* Enum)
{
	if (EnumSet.Find(Enum->GetFName())) return true;
	EnumSet.Add(Enum->GetFName());
	BeginSegment();
	O(TEXT("declare enum %s {"), *Enum->GetName());
	for (int32 index = 0; index < Enum->NumEnums(); index++)
	{
		auto Value = Enum->GetNameByIndex(index).ToString();
		if (auto pos = Value.Find(TEXT("::")))
		{
			Value = Value.Mid(pos + 2);
		}
		O(TEXT("	%s = %d,"), *Value, Enum->GetValueByIndex(index));
	}
	O(TEXT("}"));
	EndSegment();
	return true;
}

#undef O
