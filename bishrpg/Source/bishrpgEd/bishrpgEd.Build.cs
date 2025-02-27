// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class bishrpgEd : ModuleRules
{
	public bishrpgEd(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        //PCHUsage = PCHUsageMode.UseSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { "bishrpgEd/Public" });
        PrivateIncludePaths.AddRange(new string[] { "bishrpgEd/Private" });
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UnrealEd", "AssetTools", "EditorScriptingUtilities", "ContentBrowser" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
