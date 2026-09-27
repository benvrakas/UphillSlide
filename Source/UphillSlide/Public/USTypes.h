#pragma once

#include "CoreMinimal.h"
#include "USTypes.generated.h"

// What a player wears on their legs, for choosing which slide settings apply.
UENUM()
enum class EUSTier : uint8
{
	// No Blade Runners.
	None,
	// Vanilla Blade Runners, and Blade Runners from mods other than Mk+ Blade Runners.
	Mk1,
	// Mk+ Blade Runners tiers (any of their fall damage variants).
	Mk2,
	Mk3,
	Mk4,
	Mk5,
	Count UMETA(Hidden)
};

constexpr int32 USTierCount = static_cast<int32>(EUSTier::Count);

// Slide settings for one tier, as the host has them.
USTRUCT()
struct FUSTierSettings
{
	GENERATED_BODY()

	// Steepest uphill slope a slide keeps going on, in degrees above horizontal.
	UPROPERTY()
	float UphillAngleDegrees = 0.f;

	// How fast a slide runs out while going uphill, in percent of vanilla.
	UPROPERTY()
	float UphillSpeedLossPercent = 100.f;

	bool operator==(const FUSTierSettings& Other) const
	{
		return UphillAngleDegrees == Other.UphillAngleDegrees && UphillSpeedLossPercent == Other.UphillSpeedLossPercent;
	}
};

// Returns the name of Tier for logs and the Mods menu.
inline const TCHAR* USTierName(EUSTier Tier)
{
	switch (Tier)
	{
	case EUSTier::None: return TEXT("No Blade Runners");
	case EUSTier::Mk1: return TEXT("Blade Runners");
	case EUSTier::Mk2: return TEXT("Mk.2 Blade Runners");
	case EUSTier::Mk3: return TEXT("Mk.3 Blade Runners");
	case EUSTier::Mk4: return TEXT("Mk.4 Blade Runners");
	case EUSTier::Mk5: return TEXT("Mk.5 Blade Runners");
	default: return TEXT("?");
	}
}
