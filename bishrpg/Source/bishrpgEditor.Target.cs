// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class bishrpgEditorTarget : TargetRules
{
	public bishrpgEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange(new string[] { "bishrpg" });
        ExtraModuleNames.AddRange(new string[] { "bishrpgEd" });
    }
}
