// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved. 
#include "JScriptPluginPrivatePCH.h"
#include "JScriptPluginComponent.h"

//////////////////////////////////////////////////////////////////////////

UJScriptPluginComponent::UJScriptPluginComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = false;
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	SourceFilename = TEXT("JiJiWaiWai");

	Context = NULL;
}

void UJScriptPluginComponent::OnRegister()
{
	Super::OnRegister();

	if (GetWorld() && GetWorld()->WorldType != EWorldType::Editor)
	{
		Context = JScriptEngine::CreateContext();
//		Context->Expose(this->GetClass());
		Context->Expose(TEXT("This"), this);
	}
}

void UJScriptPluginComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UJScriptPluginComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Context)
	{
	}
}

void UJScriptPluginComponent::OnUnregister()
{
	if (Context)
	{
		JScriptEngine::FreeContext(Context);
		Context = nullptr;
	}

	Super::OnUnregister();
}

bool UJScriptPluginComponent::Execute(FString Code)
{
	bool bSuccess = false;
	if (Context)
	{
		Context->Execute(Code);
		bSuccess = true;
	}
	return bSuccess;
}

void UJScriptPluginComponent::Output(FString Message)
{
	UE_LOG(LogJScriptPlugin, Verbose, TEXT("%s"), *Message);
}
