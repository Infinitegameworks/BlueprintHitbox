using UnrealBuildTool;

public class BlueprintHitbox : ModuleRules
{
	public BlueprintHitbox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"Json",
			"JsonUtilities",
			"Paper2D"
		});
	}
}
