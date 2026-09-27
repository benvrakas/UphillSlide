#pragma once

#include "CoreMinimal.h"
#include "Configuration/ModConfiguration.h"
#include "USTypes.h"
#include "USConfig.generated.h"

// Uphill Slide settings, editable in-game and in Satisfactory Mod Manager: one section per tier.
UCLASS()
class UPHILLSLIDE_API UUSConfig : public UModConfiguration
{
	GENERATED_BODY()

public:
	UUSConfig();

	// Setting keys inside each tier's section.
	static const FString UphillAngleKey;
	static const FString UphillSpeedLossKey;

	// Lowest and highest accepted values.
	static constexpr float MinUphillAngle = 0.f;
	static constexpr float MaxUphillAngle = 89.f;
	static constexpr float MinUphillSpeedLoss = 0.f;
	static constexpr float MaxUphillSpeedLoss = 1000.f;

	// Returns the key of Tier's section.
	static FString GetTierSectionKey(EUSTier Tier);

	// Rebuilds the default configuration with SML's Blueprint property classes, which carry the editor widgets
	// SML's Mods menu shows. Call before SML registers the configuration.
	static void UseSMLEditorClasses();

	// Fills OutSettings with every tier's settings, clamped to the accepted range. Returns false, leaving OutSettings
	// empty, if the configuration isn't available.
	static bool ReadTierSettings(const UObject* WorldContext, TArray<FUSTierSettings>& OutSettings);
};
