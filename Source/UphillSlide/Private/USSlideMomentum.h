#pragma once

#include "CoreMinimal.h"

class UFGCharacterMovementComponent;

// Stops a slide from pulling the player below the speed they entered it with, for players whose tier has
// "Keep your entry speed" on. The kept speed fades by the slide curve, like the normal slide speed.
class FUSSlideMomentum
{
public:
	// Hooks UFGCharacterMovementComponent::GetMaxSpeed. Does nothing in editor builds or when already installed.
	static void InstallHook();

	// Sets whether Movement keeps its entry speed while sliding.
	static void SetEnabled(const UFGCharacterMovementComponent* Movement, bool bEnabled);

	// Forgets movement components that no longer exist.
	static void Prune();

	// Forgets every movement component.
	static void Reset();

private:
	// Returns the max speed Movement slides at: VanillaMaxSpeed, raised to the player's faded entry speed when enabled.
	static float AdjustMaxSpeed(const UFGCharacterMovementComponent* Movement, float VanillaMaxSpeed);
};
