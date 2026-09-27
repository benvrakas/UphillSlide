<!--
Text for the Uphill Slide page on ficsit.app (Satisfactory Mod Repository).
- "Short description" field: the line under SHORT DESCRIPTION (at most 128 characters).
- "Full description" field: everything under FULL DESCRIPTION.
- The two transparency fields: the text under NETWORK ACTIVITY TRANSPARENCY and AI USAGE TRANSPARENCY.
Replace the [bracketed] placeholders before publishing.
-->

# SHORT DESCRIPTION

Slide up steeper ramps. Set how steep you can slide, and how fast you slow down uphill, for each Blade Runners tier.

# FULL DESCRIPTION

## Uphill Slide

In vanilla Satisfactory a slide dies the moment the ground tilts up more than about **7.4°**. That's barely more than a
1 m ramp. Uphill Slide lets you pick that limit yourself, separately for each tier of Blade Runners, so better legs
carry you further up your factory.

[Screenshot or GIF: sliding up a 2 m or 4 m ramp]

### Features

- **Steepest uphill slide, per tier.** Choose how steep a slope you can keep sliding up, from 0° to 89°.
- **Uphill slowdown, per tier.** Choose how quickly a slide loses speed while going uphill: 100% is vanilla, 50%
  keeps your speed twice as long, 0% doesn't slow you down uphill at all. Flat ground and downhill slides are
  unchanged.
- **Works with [Mk+ Blade Runners](https://ficsit.app/mod/bbladerunners)** (optional). Mk.2, Mk.3, Mk.4 and Mk.5
  each get their own settings, and their reduced and no fall damage versions count as the same tier.
- **Multiplayer.** Everyone slides by the host's settings, so there's no rubber-banding on slopes.

### Default settings

| Wearing | Steepest uphill slide | Enough for |
|---|---|---|
| No Blade Runners | 7.4° (vanilla) | 1 m ramps |
| Blade Runners | 15° | 2 m ramps |
| Mk.2 Blade Runners | 20° | |
| Mk.3 Blade Runners | 27° | 4 m ramps |
| Mk.4 Blade Runners | 35° | |
| Mk.5 Blade Runners | 40° | |

Uphill slowdown starts at 100% (vanilla) for every tier.

Ramps on an 8 m foundation: 1 m is 7.1°, 2 m is 14.0°, 4 m is 26.6°. Set a tier a little above the ramp you want
to slide up.

### Settings

Open **Esc → Mods → Uphill Slide**, or the mod's config in Satisfactory Mod Manager. Each tier has its own section.
Changes apply straight away, no restart needed.

Blade Runners added by other mods (not Mk+ Blade Runners) use the **Blade Runners** settings.

### Multiplayer

- Everyone in the session needs Uphill Slide.
- The host's settings apply to every player. On a dedicated server, the server's config is used. Your own settings
  only matter in games you host.
- Works on Windows and Linux dedicated servers.

### Known limits

- A slide still needs speed. Uphill slowdown only changes how fast the slide runs out; it doesn't push you up a
  slope you reached too slowly.
- Very steep settings (past about 45°) may do nothing more, because the game doesn't let you stand on slopes that
  steep.

### Bugs and ideas

Report them on [GitHub](https://github.com/benvrakas/UphillSlide/issues). The source code is there too.

# NETWORK ACTIVITY TRANSPARENCY

Uphill Slide doesn't connect to the internet. In multiplayer, the host sends its slide settings to players through
the game's own connection. There's no telemetry or analytics.

# AI USAGE TRANSPARENCY

[Edit to match how you made the mod.]

This mod was built with the help of an AI coding assistant (Anthropic's Claude):

- **Source code:** the mod's C++ source code was written by the AI, directed, reviewed and tested in game by the
  author.
- **Mod page text:** this description was drafted by the AI and edited by the author.
- **Developer documentation** in the source folder was written by the AI.
- **Art and audio:** the mod adds no art or audio. It uses the game's own settings menu.
- **In game:** the mod doesn't use or give access to generative AI.
