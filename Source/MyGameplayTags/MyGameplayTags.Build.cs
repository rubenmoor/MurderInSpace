using UnrealBuildTool;

public class MyGameplayTags : ModuleRules
{
    public MyGameplayTags(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                  "CoreUObject"
                , "Engine"
                , "GameplayTags"
            }
        );
    }
}