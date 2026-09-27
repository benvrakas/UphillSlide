#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ModSubsystem.h"
#include "USTypes.h"
#include "USSlideSubsystem.generated.h"

class AFGCharacterPlayer;
class UCurveFloat;
class UFGCharacterMovementComponent;

// Applies each player's tier settings to their movement component. The server reads the host's configuration and
// replicates it; the server and each player's own game then apply the same values to that player.
UCLASS()
class UPHILLSLIDE_API AUSSlideSubsystem : public AModSubsystem
{
	GENERATED_BODY()

public:
	AUSSlideSubsystem();

	// Returns the subsystem for the world WorldContext belongs to, or null if it hasn't spawned yet.
	static AUSSlideSubsystem* Get(const UObject* WorldContext);

	// Returns which tier's settings apply to Character, from the Blade Runners they're wearing.
	EUSTier GetTier(const AFGCharacterPlayer* Character);

	//~ Begin AActor interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End AActor interface

private:
	// Vanilla values of a movement component this subsystem has changed, and what it last applied.
	struct FTrackedMovement
	{
		float VanillaMaxSlideAngle = 0.f;
		TWeakObjectPtr<UCurveFloat> VanillaSlopeCurve;
		EUSTier AppliedTier = EUSTier::Count;
		FUSTierSettings AppliedSettings;
	};

	// Server only. Copies the host's configuration into TierSettings if it changed.
	void RefreshSettingsFromConfig();

	// Sets Character's max slide angle and slope curve from their tier's settings.
	void ApplyTo(AFGCharacterPlayer* Character);

	// Returns a copy of Vanilla whose uphill values are multiplied by Factor, built once per curve and factor.
	UCurveFloat* GetScaledSlopeCurve(UCurveFloat* Vanilla, float Factor);

	// Puts every tracked movement component back to its vanilla values.
	void RestoreAll();

	// The host's settings, indexed by EUSTier. Empty until the server has read its configuration.
	UPROPERTY(Replicated)
	TArray<FUSTierSettings> TierSettings;

	// Slope curves built by GetScaledSlopeCurve, kept alive while in use.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCurveFloat>> ScaledCurves;

	// GetScaledSlopeCurve's cache: (vanilla curve, factor in thousandths) to built curve.
	TMap<TPair<const UCurveFloat*, int32>, UCurveFloat*> ScaledCurveLookup;

	TMap<TWeakObjectPtr<UFGCharacterMovementComponent>, FTrackedMovement> Tracked;

	// Blade Runners classes whose tier has been logged.
	TSet<FString> LoggedClasses;

	double NextConfigReadTime = 0.0;
	bool bLoggedWaitingForSettings = false;
	bool bLoggedVanillaCurve = false;
	bool bLoggedMissingCurve = false;
};
