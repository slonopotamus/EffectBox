using UnrealBuildTool;

public class EffectBox : ModuleRules
{
	public EffectBox(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"RHI",
				"RenderCore",
				"Slate",
				"SlateCore",
				"UMG",
			}
		);
	}
}
