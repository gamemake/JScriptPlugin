using UnrealBuildTool;
using System.IO;
using System;

public class JScriptV8Plugin : ModuleRules
{
    protected string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty")); }
    }

    public JScriptV8Plugin(TargetInfo Target)
    {
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(ThirdPartyPath, "v8", "include"),
            Path.Combine("V8", "Private")
        });

        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine("V8", "Public")
        });

        PublicDependencyModuleNames.AddRange(new string[] 
        { 
            "Core", "CoreUObject", "Engine"
        });

        bEnableExceptions = true;

        LoadV8(Target);
    }

    private bool LoadV8(TargetInfo Target)
    {
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib");

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                LibrariesPath = Path.Combine(LibrariesPath, "Win64");
            }
            else
            {
                LibrariesPath = Path.Combine(LibrariesPath, "Win32");
            }

            if (Target.Configuration == UnrealTargetConfiguration.Debug)
            {
                LibrariesPath = Path.Combine(LibrariesPath, "Release");
            }
            else
            {
                LibrariesPath = Path.Combine(LibrariesPath, "Debug");
            }

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_0.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_1.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_2.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_3.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_libbase.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_libplatform.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_nosnapshot.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_libsampler.lib"));
            PublicAdditionalLibraries.Add("Shlwapi.lib");
            PublicAdditionalLibraries.Add("Shlwapi.lib");

            Definitions.Add(string.Format("WITH_V8=1"));
            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "Android");
            PublicLibraryPaths.Add(Path.Combine(LibrariesPath, "ARMv7"));
            PublicLibraryPaths.Add(Path.Combine(LibrariesPath, "ARM64"));
            PublicLibraryPaths.Add(Path.Combine(LibrariesPath, "x86"));
            PublicLibraryPaths.Add(Path.Combine(LibrariesPath, "x64"));

            PublicAdditionalLibraries.Add("v8_base");
            PublicAdditionalLibraries.Add("v8_libbase");
            PublicAdditionalLibraries.Add("v8_libplatform");
            PublicAdditionalLibraries.Add("v8_nosnapshot");
            PublicAdditionalLibraries.Add("v8_libsampler");

            Definitions.Add(string.Format("WITH_V8=1"));

            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "Linux");
            PublicLibraryPaths.Add(Path.Combine(LibrariesPath, "x64"));

            PublicAdditionalLibraries.Add("v8_base");
            PublicAdditionalLibraries.Add("v8_libbase");
            PublicAdditionalLibraries.Add("v8_libplatform");
            PublicAdditionalLibraries.Add("v8_nosnapshot");
            PublicAdditionalLibraries.Add("v8_libsampler");

            Definitions.Add(string.Format("WITH_V8=1"));
            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "Mac", "x64");
            PublicLibraryPaths.Add(LibrariesPath);

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath,"libv8_base.a"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath,"libv8_libbase.a"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath,"libv8_libplatform.a"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath,"libv8_nosnapshot.a"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libsampler.a"));

            Definitions.Add(string.Format("WITH_V8=1"));
            return true;
        }

        Definitions.Add(string.Format("WITH_V8=0"));
        return false;
    }
}
