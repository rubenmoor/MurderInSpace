using UnrealBuildTool;

public class MurderInSpaceTarget : TargetRules
{
	public MurderInSpaceTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange( new [] { "MurderInSpace" } );
		AdditionalCompilerArguments = "-Wno-deprecated-enum-enum-conversion";
		RegisterModulesCreatedByRider();
	}

	private void RegisterModulesCreatedByRider()
	{
		ExtraModuleNames.AddRange(new string[] { "MyGameplayTags" });
	}
}
