// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
    public class JScriptEditorPlugin : ModuleRules
    {
        public JScriptEditorPlugin(TargetInfo Target)
        {
            PublicIncludePaths.AddRange(
                new string[] {					
					//"Programs/UnrealHeaderTool/Public",
					// ... add other public include paths required here ...
				}
                );

            PrivateIncludePaths.AddRange(
                new string[] {
					// ... add other private include paths required here ...
				}
                );

            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "InputCore",
                    "UnrealEd",
                    "AssetTools",
                    "ClassViewer",
                    "KismetCompiler",
                    "Kismet",
                    "BlueprintGraph",
                    "LevelEditor",
                    "Slate",
                    "SlateCore",
                    "EditorStyle",

					// ... add other public dependencies that you statically link with here ...
                    "JScriptPlugin",
                    "JScriptV8Plugin"
                }
                );

            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
					// ... add private dependencies that you statically link with here ...
                }
                );

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
                {
					// ... add any modules that your module loads dynamically here ...
				}
                );
        }
    }
}