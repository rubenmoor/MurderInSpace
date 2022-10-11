// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MurderInSpace : ModuleRules
{
	public MurderInSpace(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
			{ "Core"
			, "CoreUObject"
			, "Engine"
			, "InputCore"
			, "Niagara"
			, "UMG",
			});

		PrivateDependencyModuleNames.AddRange(new string[]
			{ "OnlineSubsystem"
			, "OnlineSubsystemUtils"
			, "Slate"
			, "SlateCore"
			});
		
		// DynamicallyLoadedModuleNames.AddRange( new string[]
		// 	{ "OnlineSubsystemNull"
		// 	});

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
