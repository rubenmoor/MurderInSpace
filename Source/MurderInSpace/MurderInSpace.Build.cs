// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MurderInSpace : ModuleRules
{
	public MurderInSpace(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new []
			{ "Core"
			, "CommonUI"
			, "CoreUObject"
			, "UE5Coro"
			, "UE5CoroGAS"
			, "Engine"
			, "FastNoiseGenerator"
			, "FastNoise"
			, "GeometryScriptingCore"
			, "GeometryFramework"
			, "InputCore"
			, "LoadingScreen"
			, "Niagara"
			, "PhysicsCore"
			, "UMG"
			});

		PrivateDependencyModuleNames.AddRange(new []
			{ "Chaos"
			, "EnhancedInput"
			, "GameplayAbilities"
			, "GameplayTags"
			, "GameplayTasks"
			, "OnlineSubsystem"
			, "OnlineSubsystemNull"
			, "OnlineSubsystemUtils"
			, "ProceduralMeshComponent"
			, "RealtimeMeshComponent"
			, "Slate"
			, "SlateCore"
			});

		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.Add("UnrealEd");
		}
		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
