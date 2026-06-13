# Mod: Experience System Changes

## 1. Experience Multiplier (x4)

**File:** `Source/player.cpp`
**Location:** Constant `ExperienceMultiplier` defined above the `AddPlrExperience` function.

- Added a `constexpr int ExperienceMultiplier = 4` constant that multiplies all experience gained.
- Applies to both single-player and multiplayer.
- To change the multiplier, update the value of `ExperienceMultiplier` and recompile.

**Resulting formula:**

```
clampedExp = exp * ExperienceMultiplier * (1 + (monster_lvl - player_lvl) / 10)
```

## 2. Shared Experience for All Players

**File:** `Source/player.cpp`
**Location:** Function `AddPlrMonstExper`.

- Replaced the `pmask`-based distribution (who hit the monster) with `gbActivePlayers` (all connected players).
- All connected players now receive the same portion of experience when a monster dies, regardless of who hit it.
- No visible change in single-player (`gbActivePlayers = 1`).

**Before:**

- Only players who hit the monster received XP.
- XP was divided among participants.

**After:**

- All connected players receive XP.
- XP is divided by `gbActivePlayers`.
