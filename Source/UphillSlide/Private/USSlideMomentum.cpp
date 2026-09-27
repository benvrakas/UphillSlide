#include "USSlideMomentum.h"
#include "UphillSlide.h"
#include "Curves/CurveFloat.h"
#include "FGCharacterMovementComponent.h"
#include "Patching/NativeHookManager.h"

namespace
{
	struct FMomentumState
	{
		bool bEnabled = false;
		bool bWasSliding = false;
		// Horizontal speed when the current slide started, and the slide curve's value at that moment.
		float EntrySpeed = 0.f;
		float EntryCurveValue = 0.f;
	};

	TMap<TWeakObjectPtr<const UFGCharacterMovementComponent>, FMomentumState> States;
	bool bHookInstalled = false;
}

void FUSSlideMomentum::InstallHook()
{
#if WITH_EDITOR
	UE_LOG(LogUphillSlide, Log, TEXT("Editor build: slide momentum hook not installed"));
#else
	if (bHookInstalled)
	{
		return;
	}
	bHookInstalled = true;
	SUBSCRIBE_UOBJECT_METHOD(UFGCharacterMovementComponent, GetMaxSpeed, [](auto& Scope, const UFGCharacterMovementComponent* Self)
	{
		const float VanillaMaxSpeed = Scope(Self);
		const float Adjusted = AdjustMaxSpeed(Self, VanillaMaxSpeed);
		if (Adjusted != VanillaMaxSpeed)
		{
			Scope.Override(Adjusted);
		}
	});
	UE_LOG(LogUphillSlide, Log, TEXT("Hook installed: UFGCharacterMovementComponent::GetMaxSpeed"));
#endif
}

void FUSSlideMomentum::SetEnabled(const UFGCharacterMovementComponent* Movement, bool bEnabled)
{
	if (bEnabled)
	{
		States.FindOrAdd(Movement).bEnabled = true;
	}
	else if (FMomentumState* State = States.Find(Movement))
	{
		State->bEnabled = false;
		State->bWasSliding = false;
	}
}

void FUSSlideMomentum::Prune()
{
	for (auto It = States.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
	}
}

void FUSSlideMomentum::Reset()
{
	States.Reset();
}

float FUSSlideMomentum::AdjustMaxSpeed(const UFGCharacterMovementComponent* Movement, float VanillaMaxSpeed)
{
	FMomentumState* State = States.Find(Movement);
	if (!State || !State->bEnabled)
	{
		return VanillaMaxSpeed;
	}
	if (!Movement->IsSliding())
	{
		State->bWasSliding = false;
		return VanillaMaxSpeed;
	}

	const UCurveFloat* SlideCurve = Movement->mSlideCurve;
	const float CurveValue = SlideCurve ? SlideCurve->GetFloatValue(Movement->mSlideTime) : 1.f;
	if (!State->bWasSliding)
	{
		State->bWasSliding = true;
		State->EntrySpeed = Movement->Velocity.Size2D();
		State->EntryCurveValue = CurveValue;
		UE_LOG(LogUphillSlide, Verbose, TEXT("Slide started at %.0f cm/s (vanilla slide speed %.0f)"), State->EntrySpeed, VanillaMaxSpeed);
	}
	if (State->EntryCurveValue <= KINDA_SMALL_NUMBER)
	{
		return VanillaMaxSpeed;
	}

	// Never pulls the player below their current speed while that is under the faded entry speed.
	const float FadedEntrySpeed = State->EntrySpeed * CurveValue / State->EntryCurveValue;
	return FMath::Max(VanillaMaxSpeed, FMath::Min(static_cast<float>(Movement->Velocity.Size()), FadedEntrySpeed));
}
