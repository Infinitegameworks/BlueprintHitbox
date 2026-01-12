using UnrealBuildTool;

public class BlueprintHitboxEditor : ModuleRules
{
	public BlueprintHitboxEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"BlueprintHitbox",
			"Slate",
			"SlateCore",
			"EditorStyle",
			"Paper2D",
			"Json",
			"JsonUtilities",
			"AssetTools",
			"ContentBrowser",
			"InputCore",
			"PropertyEditor",
			"EditorFramework",
			"ToolMenus",
			"DesktopPlatform"
		});
	}
}
