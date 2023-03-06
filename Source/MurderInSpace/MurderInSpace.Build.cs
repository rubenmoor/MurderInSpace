// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MurderInSpace : ModuleRules
{
	public MurderInSpace(ReadOnlyTargetRules Target) : base(Target)
	{
		//CppStandard = CppStandardVersion.Cpp20;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new []
			{ "Core"
			, "CommonUI"
			, "CoreUObject"
			, "Engine"
			, "InputCore"
			, "Niagara"
			, "UMG"
			});

		PrivateDependencyModuleNames.AddRange(new []
			{ "Chaos"
			, "EnhancedInput"
			, "GeometryScriptingCore"
			, "GameplayTags"
			, "MeshDescription"
			, "MeshConversion"
			, "OnlineSubsystem"
			, "OnlineSubsystemNull"
			, "OnlineSubsystemUtils"
			, "Slate"
			, "SlateCore"
			, "StaticMeshDescription"
			});
		
		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
