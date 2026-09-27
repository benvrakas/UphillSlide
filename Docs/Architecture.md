# Architecture

## How vanilla sliding decides "too steep"

Found by disassembling `FactoryGameSteam-FactoryGame-Win64-Shipping.dll` (game CL 502094). The starter
project's `.cpp` files are stubs, so none of this is visible in source.

`UFGCharacterMovementComponent::CanSlide()` and `CanStartSlide()` both do:

```
angle = acos( dot( Velocity.GetSafeNormal(), GravityDirection ) )   // radians; flat = pi/2, uphill > pi/2
if angle > mMaxSlideAngle: no slide
```

- `mMaxSlideAngle` (offset `0x15BC`) is 1.65 rad in the C++ constructor, but the player Blueprint overrides it to
  **1.70 rad**: pi/2 + 0.129 rad, about **7.40 degrees uphill** (read in-game from the log). Downhill angles are always
  under pi/2, so they always pass.
- `CanSlide` also ends the slide once the horizontal speed of `mBaseVelocity` drops to `MaxWalkSpeedCrouched`.
  `CanStartSlide` needs more than 1.1 x `MaxWalkSpeed`.

`TickSlide(delta)` does:

```
mSlideTime = max(0, mSlideTime + mSlopeCurve->GetFloatValue(angle) * delta)   // same angle as above, radians
```

and `GetMaxSpeed()`, while sliding, uses `mSlideCurve->GetFloatValue(mSlideTime)` times the sprint speed
(including the Blade Runners' `GetAdjustedMaxSpeed`). So `mSlopeCurve` sets how fast a slide runs out at each
slope. The slide timer runs backwards on steep downhill slopes, which is why they keep a slide going.

The vanilla curve is `/Game/FactoryGame/Character/Player/SlopeCurveMultiplier` with keys (angle rad, rate):
`(0.934, -0.606) (1.344, 0.089) (1.488, 0.857) (1.570, 1.000)`. Nothing is keyed above flat, so every uphill angle
holds the flat rate of 1.0. Uphill slides slow down exactly as fast as flat ones; they only end sooner because of the
angle limit.

While sliding, `CalcVelocity` sets the speed to `GetMaxSpeed()` along the current direction, and `GetMaxSpeed` eases
the current speed toward `mSlideCurve(mSlideTime) x sprint speed`. Nothing else in the slide code depends on slope.

## What the mod changes

Per player, on the server and on that player's own game:

- `mMaxSlideAngle` = pi/2 + the tier's "steepest uphill slide" in radians.
- `mSlopeCurve` = the vanilla curve, or a per-factor copy (`GetScaledSlopeCurve`). Keys below flat are copied as
  they are. Above flat, the copy samples the vanilla curve times the tier's "uphill slowdown" factor at flat + 1
  degree, at any vanilla keys beyond that, and at pi, so the scaling holds for every uphill angle. Between flat (vanilla
  value) and flat + 1 degree it blends linearly, so tiny vertical jitter on level floors barely changes the rate.
  At 0% the slide timer stops advancing uphill and the slide keeps its speed.

Both fields are private `UPROPERTY`s, reached through the `Accessor` access transformers in
`Config/AccessTransformers.ini`.

## Why polling instead of hooks

`AUSSlideSubsystem` ticks every 0.1 s and sets the values only when they differ. The alternative, hooking
`AFGJumpingStilts::Equip/UnEquip`, misses respawns, joins, config edits and equipment that a Blueprint subclass
equips differently. Polling handles all of those, and the work is a few comparisons per player.

## Multiplayer

Character movement is simulated on the owning client and re-run on the server. If the two disagree on
`mMaxSlideAngle`, the server corrects the client and the player rubber-bands. So:

- Only the server reads the configuration (`RefreshSettingsFromConfig`, every second) and replicates
  `TierSettings`. Clients never use their own config values.
- Each machine applies those settings to the characters it simulates: the server to all of them, and a client
  only to its own (`HasAuthority() || IsLocallyControlled()`).
- Until the settings arrive, a client leaves slides vanilla.

## Tiers

`GetTier` looks for an `AFGJumpingStilts` among the character's active equipment:

- none: `EUSTier::None`
- class path under `/bbladerunners/` whose name starts with `Mk2`..`Mk5`: that tier. Mk+ Blade Runners
  (Blueprint-only mod, `bbladerunners` 1.2.0) has `Mk2_stilts`, `Mk2Reduced_stilts`, `Mk2NoDamage_stilts`,
  `Mk3_stilts`, `Mk3NoDamage_stilts`, `Mk4_stilts` and `Mk5_stilts` under `/bbladerunners/items/...`. The fall damage variants share their tier.
- anything else, including vanilla `BP_JumpingStilts` and other mods' Blade Runners: `EUSTier::Mk1`.

Mk+ Blade Runners is an optional plugin dependency (`"Optional": true` in the `.uplugin`). Nothing links against it;
the tier comes from the class name, so the mod works the same with or without it.
