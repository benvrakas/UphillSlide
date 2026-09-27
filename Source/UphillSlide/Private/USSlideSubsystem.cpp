#include "USSlideSubsystem.h"
#include "UphillSlide.h"
#include "USConfig.h"
#include "USSlideMomentum.h"
#include "Curves/CurveFloat.h"
#include "EngineUtils.h"
#include "Equipment/FGEquipment.h"
#include "FGCharacterMovementComponent.h"
#include "FGCharacterPlayer.h"
#include "FGJumpingStilts.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Subsystem/SubsystemActorManager.h"

namespace
{
	constexpr float ApplyInterval = 0.1f;
	constexpr double ConfigReadInterval = 1.0;

	// Slope angle of flat ground: the angle between a level velocity and straight down.
	constexpr float FlatSlopeAngle = UE_HALF_PI;

	// Number of steps a scaled slope curve samples the vanilla one at, from straight down to straight up (half a degree each).
	constexpr int32 SlopeCurveSamples = 360;

	// Package path prefix of Mk+ Blade Runners' classes.
	const TCHAR* MkPlusPathPrefix = TEXT("/bbladerunners/");

	// Returns the player's name, or the character's object name if it has no player state.
	FString DescribePlayer(const AFGCharacterPlayer* Character)
	{
		const APlayerState* PlayerState = Character ? Character->GetPlayerState() : nullptr;
		return PlayerState ? PlayerState->GetPlayerName() : GetNameSafe(Character);
	}

	// Returns Curve's keys as "(time, value)" pairs.
	FString DescribeKeys(const FRichCurve& Curve)
	{
		TArray<FString> Parts;
		for (const FRichCurveKey& Key : Curve.GetConstRefOfKeys())
		{
			Parts.Add(FString::Printf(TEXT("(%.3f, %.3f)"), Key.Time, Key.Value));
		}
		return FString::Join(Parts, TEXT(" "));
	}
}

AUSSlideSubsystem::AUSSlideSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnOnServer_Replicate;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickInterval = ApplyInterval;
}

AUSSlideSubsystem* AUSSlideSubsystem::Get(const UObject* WorldContext)
{
	UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	USubsystemActorManager* Manager = World ? World->GetSubsystem<USubsystemActorManager>() : nullptr;
	return Manager ? Manager->GetSubsystemActor<AUSSlideSubsystem>() : nullptr;
}

void AUSSlideSubsystem::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogUphillSlide, Log, TEXT("Slide subsystem started (%s)"), HasAuthority() ? TEXT("server") : TEXT("client"));
	if (HasAuthority())
	{
		RefreshSettingsFromConfig();
	}
}

void AUSSlideSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RestoreAll();
	Super::EndPlay(EndPlayReason);
}

void AUSSlideSubsystem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AUSSlideSubsystem, TierSettings);
}

void AUSSlideSubsystem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now >= NextConfigReadTime)
	{
		NextConfigReadTime = Now + ConfigReadInterval;
		if (HasAuthority())
		{
			RefreshSettingsFromConfig();
		}
		for (auto It = Tracked.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				It.RemoveCurrent();
			}
		}
		FUSSlideMomentum::Prune();
	}

	if (TierSettings.Num() != USTierCount)
	{
		if (!bLoggedWaitingForSettings)
		{
			bLoggedWaitingForSettings = true;
			UE_LOG(LogUphillSlide, Log, TEXT("Waiting for the host's settings (have %d of %d tiers); slides stay vanilla until then"), TierSettings.Num(), USTierCount);
		}
		return;
	}

	for (TActorIterator<AFGCharacterPlayer> It(GetWorld()); It; ++It)
	{
		AFGCharacterPlayer* Character = *It;
		if (Character->HasAuthority() || Character->IsLocallyControlled())
		{
			ApplyTo(Character);
		}
	}
}

void AUSSlideSubsystem::RefreshSettingsFromConfig()
{
	TArray<FUSTierSettings> NewSettings;
	if (!UUSConfig::ReadTierSettings(this, NewSettings) || NewSettings == TierSettings)
	{
		return;
	}
	TierSettings = MoveTemp(NewSettings);
	for (int32 Index = 0; Index < TierSettings.Num(); ++Index)
	{
		UE_LOG(LogUphillSlide, Log, TEXT("Settings: %s: uphill up to %.2f degrees, speed loss %.0f%%, keep entry speed %s"),
			USTierName(static_cast<EUSTier>(Index)), TierSettings[Index].UphillAngleDegrees, TierSettings[Index].SpeedLossPercent,
			TierSettings[Index].bKeepEntrySpeed ? TEXT("on") : TEXT("off"));
	}
}

EUSTier AUSSlideSubsystem::GetTier(const AFGCharacterPlayer* Character)
{
	const AFGJumpingStilts* Stilts = nullptr;
	for (AFGEquipment* Equipment : Character->GetActiveEquipments())
	{
		Stilts = Cast<AFGJumpingStilts>(Equipment);
		if (Stilts)
		{
			break;
		}
	}
	if (!Stilts)
	{
		return EUSTier::None;
	}

	const UClass* Class = Stilts->GetClass();
	EUSTier Tier = EUSTier::Mk1;
	const FString Name = Class->GetName();
	if (Class->GetPathName().StartsWith(MkPlusPathPrefix) && Name.Len() > 2 && Name.StartsWith(TEXT("Mk")) && FChar::IsDigit(Name[2]))
	{
		const int32 Mark = Name[2] - TEXT('0');
		if (Mark >= static_cast<int32>(EUSTier::Mk2) && Mark <= static_cast<int32>(EUSTier::Mk5))
		{
			Tier = static_cast<EUSTier>(Mark);
		}
	}

	const FString ClassPath = Class->GetPathName();
	if (!LoggedClasses.Contains(ClassPath))
	{
		LoggedClasses.Add(ClassPath);
		UE_LOG(LogUphillSlide, Log, TEXT("Blade Runners class %s uses the %s settings"), *ClassPath, USTierName(Tier));
	}
	return Tier;
}

void AUSSlideSubsystem::ApplyTo(AFGCharacterPlayer* Character)
{
	UFGCharacterMovementComponent* Movement = Character->GetFGMovementComponent();
	if (!Movement)
	{
		return;
	}

	FTrackedMovement* State = Tracked.Find(Movement);
	if (!State)
	{
		State = &Tracked.Add(Movement);
		State->VanillaMaxSlideAngle = Movement->GetmMaxSlideAngle();
		State->VanillaSlopeCurve = Movement->GetmSlopeCurve();
		UE_LOG(LogUphillSlide, Log, TEXT("Tracking %s (%s): vanilla max slide angle %.4f rad (%.2f degrees uphill), slope curve %s"),
			*DescribePlayer(Character), Character->HasAuthority() ? TEXT("server") : TEXT("own client"), State->VanillaMaxSlideAngle,
			FMath::RadiansToDegrees(State->VanillaMaxSlideAngle - FlatSlopeAngle), *GetPathNameSafe(State->VanillaSlopeCurve.Get()));
		if (const UCurveFloat* VanillaCurve = State->VanillaSlopeCurve.Get(); VanillaCurve && !bLoggedVanillaCurve)
		{
			bLoggedVanillaCurve = true;
			UE_LOG(LogUphillSlide, Log, TEXT("Vanilla slope curve keys (slope angle rad, slide time rate): %s"), *DescribeKeys(VanillaCurve->FloatCurve));
		}
	}

	const EUSTier Tier = GetTier(Character);
	const FUSTierSettings& Settings = TierSettings[static_cast<int32>(Tier)];
	const float MaxSlideAngle = FlatSlopeAngle + FMath::DegreesToRadians(Settings.UphillAngleDegrees);
	const float SpeedLossFactor = Settings.SpeedLossPercent / 100.f;

	UCurveFloat* VanillaCurve = State->VanillaSlopeCurve.Get();
	UCurveFloat* SlopeCurve = VanillaCurve;
	if (!FMath::IsNearlyEqual(SpeedLossFactor, 1.f))
	{
		if (VanillaCurve)
		{
			SlopeCurve = GetScaledSlopeCurve(VanillaCurve, SpeedLossFactor);
		}
		else if (!bLoggedMissingCurve)
		{
			bLoggedMissingCurve = true;
			UE_LOG(LogUphillSlide, Warning, TEXT("%s has no slope curve; speed loss can't be changed"), *DescribePlayer(Character));
		}
	}

	if (Movement->GetmMaxSlideAngle() != MaxSlideAngle)
	{
		Movement->SetmMaxSlideAngle(MaxSlideAngle);
	}
	if (Movement->GetmSlopeCurve() != SlopeCurve)
	{
		Movement->SetmSlopeCurve(SlopeCurve);
	}
	FUSSlideMomentum::SetEnabled(Movement, Settings.bKeepEntrySpeed);

	if (State->AppliedTier != Tier || !(State->AppliedSettings == Settings))
	{
		State->AppliedTier = Tier;
		State->AppliedSettings = Settings;
		UE_LOG(LogUphillSlide, Log, TEXT("%s (%s) now uses %s: uphill up to %.2f degrees (max slide angle %.4f rad), speed loss %.0f%%, keep entry speed %s"),
			*DescribePlayer(Character), Character->HasAuthority() ? TEXT("server") : TEXT("own client"), USTierName(Tier),
			Settings.UphillAngleDegrees, MaxSlideAngle, Settings.SpeedLossPercent, Settings.bKeepEntrySpeed ? TEXT("on") : TEXT("off"));
	}
}

UCurveFloat* AUSSlideSubsystem::GetScaledSlopeCurve(UCurveFloat* Vanilla, float Factor)
{
	const TPair<const UCurveFloat*, int32> Key(Vanilla, FMath::RoundToInt(Factor * 1000.f));
	if (UCurveFloat* const* Found = ScaledCurveLookup.Find(Key))
	{
		return *Found;
	}

	UCurveFloat* Scaled = DuplicateObject<UCurveFloat>(Vanilla, this);
	const FRichCurve& Source = Vanilla->FloatCurve;
	FRichCurve& Target = Scaled->FloatCurve;
	Target.Reset();

	// Positive rates (the slide losing speed) are scaled; negative rates (steep downhill, gaining speed) stay vanilla.
	for (int32 Sample = 0; Sample <= SlopeCurveSamples; ++Sample)
	{
		const float Angle = UE_PI * Sample / SlopeCurveSamples;
		const float Rate = Source.Eval(Angle);
		Target.SetKeyInterpMode(Target.AddKey(Angle, Rate > 0.f ? Rate * Factor : Rate), RCIM_Linear);
	}

	ScaledCurves.Add(Scaled);
	ScaledCurveLookup.Add(Key, Scaled);
	auto RateAt = [&Target](float DegreesUphill) { return Target.Eval(FlatSlopeAngle + FMath::DegreesToRadians(DegreesUphill)); };
	UE_LOG(LogUphillSlide, Log, TEXT("Built slope curve for speed loss %.0f%% from %s: slide time rate %.3f at 30 degrees down, %.3f at 10 down, %.3f flat, %.3f at 10 up, %.3f at 30 up"),
		Factor * 100.f, *GetPathNameSafe(Vanilla), RateAt(-30.f), RateAt(-10.f), RateAt(0.f), RateAt(10.f), RateAt(30.f));
	return Scaled;
}

void AUSSlideSubsystem::RestoreAll()
{
	int32 Restored = 0;
	for (const TPair<TWeakObjectPtr<UFGCharacterMovementComponent>, FTrackedMovement>& Pair : Tracked)
	{
		if (UFGCharacterMovementComponent* Movement = Pair.Key.Get())
		{
			Movement->SetmMaxSlideAngle(Pair.Value.VanillaMaxSlideAngle);
			Movement->SetmSlopeCurve(Pair.Value.VanillaSlopeCurve.Get());
			++Restored;
		}
	}
	Tracked.Reset();
	FUSSlideMomentum::Reset();
	UE_LOG(LogUphillSlide, Log, TEXT("Restored vanilla slide values on %d players"), Restored);
}
