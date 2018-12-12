// Fill out your copyright notice in the Description page of Project Settings.
using System.IO;
using UnrealBuildTool;

public class MyProjectAndroid : ModuleRules
{
	public MyProjectAndroid(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] 
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore" ,
                "VaRestPlugin" ,
                //"TextureCompressor",
                "ProceduralMeshComponent",
                "ImageCore",
                //"TargetPlatform",
                "FBX",
                "Json",
                "JsonUtilities",
                "HTTP"
            }
        );

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        PublicIncludePaths.AddRange(new string[]
            {
                Path.Combine(ModuleDirectory,"Model"),
                Path.Combine(ModuleDirectory,"AsyncTask"),
                Path.Combine(ModuleDirectory,"Download"),
                Path.Combine(ModuleDirectory,"Upload"),
                Path.Combine(ModuleDirectory,"Struct")
            }
         );

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore"});
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
