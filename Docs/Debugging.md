# Debugging

Everything logs to **`LogUphillSlide`** in `%LOCALAPPDATA%\FactoryGame\Saved\Logs\FactoryGame.log`:

```powershell
Select-String LogUphillSlide "$env:LOCALAPPDATA\FactoryGame\Saved\Logs\FactoryGame.log"
```

## Expected sequence

```
Uphill Slide module starting
Config: 24 of 24 settings/sections use SML's editor widgets
Hook installed: UFGCharacterMovementComponent::GetMaxSpeed
Slide subsystem started (server)                       (or "client" when joining someone)
Settings: <tier>: uphill up to X degrees, speed loss Y%, keep entry speed on/off      (x6; server only, and again after each config change)
Tracking <player> (server): vanilla max slide angle 1.7000 rad (7.40 degrees uphill), slope curve /Game/...
Vanilla slope curve keys (slope angle rad, slide time rate): (t, v) ...
Blade Runners class /Game/.../BP_JumpingStilts.BP_JumpingStilts_C uses the Blade Runners settings
<player> (server) now uses Blade Runners: uphill up to 15.00 degrees (max slide angle 1.8326 rad), speed loss 100%, keep entry speed off
```

## Symptom → first thing to check

| Symptom | Look for |
|---|---|
| No `LogUphillSlide` lines | Module didn't load; check SML's mod list in the log |
| Settings never logged | `Config: settings for ... missing` or no `Settings:` lines: the config didn't register |
| Client: `Waiting for the host's settings` and nothing after | Host doesn't have the mod, or the subsystem didn't replicate |
| Vanilla angle isn't 1.70 rad | The game changed the default; the mod still works, but the "vanilla is about 7.4" text is wrong |
| Wrong tier for Mk+ Blade Runners | The `Blade Runners class ... uses ...` line shows the class path; the naming rule is in `GetTier` |
| Still stops at ~7.4 degrees | Is `now uses ...` logged for that player on **both** server and own client? |
| Rubber-banding on slopes (multiplayer) | Server and client applied different values; compare their `now uses` lines |
| Bhop speed still lost when sliding | "Keep your entry speed" on for that tier? With `Log LogUphillSlide Verbose`, each slide logs `Slide started at N cm/s (vanilla slide speed M)` |
| Slide dies fast | Lower "Speed lost while sliding"; the `Built slope curve` line shows the resulting rate at a few slopes |
