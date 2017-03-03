#pragma once

class FJScriptDefinition
{
public:
	FJScriptDefinition();
	~FJScriptDefinition();

	static FString GetClassName(UClass* Class);
	static FString GetStructName(UScriptStruct* Struct);
	static FString GetEnumName(UEnum* Enum);

	bool Export(const FString& ExportToPath);

private:
	void BeginSegment();
	void Out(const FString& line);
	void EndSegment();

	bool ExportStruct(UScriptStruct* Struct);
	bool ExportClass(UClass* Class, bool IsInterface);
	bool ExportEnum(UEnum* Enum);
	FString GetTypeName(UProperty* Property);
	FString GetFuncParamDecls(UFunction* Function, FString& ReturnType, bool NoNames);

	TSet<FName> StructSet;
	TSet<FName> ClassSet;
	TSet<FName> EnumSet;

	FString OutText;
	typedef TArray<FString> FSegment;
	TArray<FSegment> Segments;
};