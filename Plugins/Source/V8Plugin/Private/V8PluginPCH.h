// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreUObject.h"
#include "ModuleManager.h"
#include "Engine.h"

#if WITH_EDITOR
#include "UnrealEd.h"
#endif

#pragma warning( disable: 4191 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4946 )
#include <v8.h>
#include <libplatform/libplatform.h>

DECLARE_LOG_CATEGORY_EXTERN(LogV8Plugin, Log, All);

