# Cooldown System — Sistema de enfriamiento por hechizo

## Objetivo

Implementar un sistema de cooldown por hechizo que funcione en sincronía con el game loop del motor: se congela al pausar (Escape / pérdida de foco) y escala con la velocidad del juego (Normal/Fast/Faster/Fastest). El cooldown aplica a Spell y Staff charges, NO a Scrolls.

## Concepto: Game Ticks

El sistema se basa en un contador monotónico de **game ticks** (`gGameTicks`), no en tiempo real (`SDL_GetTicks()`).

```cpp
// Source/diablo.cpp — diablo.h
extern uint32_t gGameTicks;  // contador global, arranca en 0
```

**Cuándo se incrementa:** Dentro de `GameLogic()`, pero **después** del chequeo de pausa:

```cpp
void GameLogic()
{
    if (!ProcessInput()) {   // retorna false si PauseMode == 2
        return;               // gGameTicks NO se incrementa
    }
    gGameTicks++;             // solo avanza cuando el juego NO está pausado
    // ... ProcessPlayers, ProcessMonsters, etc.
}
```

Esto significa:
- **Pausa (Escape):** `PauseMode = 2` → `ProcessInput()` retorna `false` → `gGameTicks` se congela
- **Pérdida de foco:** mismo mecanismo (`diablo_focus_pause()` setea `PauseMode = 2`)
- **Velocidad:** `gGameTicks` avanza una vez por `GameLogic()`. A mayor `nTickRate`, más ticks por segundo → cooldowns pasan más rápido en tiempo real

### Relación ticks ↔ tiempo real

```
ticksPerSecond = 1000 / gnTickDelay = nTickRate
tiempoReal(seg) = ticksRestantes / ticksPerSecond
```

| Velocidad | nTickRate | gnTickDelay (ms) | Ticks/seg |
|---|---|---|---|
| Normal | 20 | 50 | 20 |
| Fast | 30 | 33 | 30 |
| Faster | 40 | 25 | 40 |
| Fastest | 50 | 20 | 50 |

## Flujo del cooldown

### 1. Datos: `sCooldown` + `sCooldownReduction` en SpellData

Cada hechizo tiene dos campos en `Source/spelldat.h`:

- `uint16_t sCooldown` — duración base en **game ticks** (línea 235). Valor `0` = sin cooldown.
- `uint8_t sCooldownReduction` — porcentaje de reducción por spell level (línea 236). Valor `0` = sin reducción.

### 2. Cooldown efectivo: `GetEffectiveCooldown()` (Source/spelldat.h)

El cooldown real depende del nivel del spell:

```
effectiveCd = sCooldown * (100 - sCooldownReduction * (spellLevel - 1)) / 100
```

- Spell level 1: cooldown base (100%)
- Cada nivel adicional reduce un `sCooldownReduction`% del cooldown base
- Mínimo 0

### 3. Registro: `DoSpell()` (Source/player.cpp:1071-1076)

El cooldown se registra **cuando el hechizo efectivamente se ejecuta**, no cuando el jugador hace click. Esto previene cooldowns falsos si la animación se cancela.

```cpp
if (IsAnyOf(player.executedSpell.spellType, SpellType::Spell, SpellType::Charges)) {
    uint16_t cd = GetSpellData(player.executedSpell.spellId).sCooldown;
    if (cd > 0) {
        player.spellCooldownStart[static_cast<int>(player.executedSpell.spellId)] = gGameTicks;
    }
}
```

- `spellCooldownStart[64]` en `Player` (`Source/player.h:342`) almacena el game tick de inicio
- Solo aplica a `SpellType::Spell` y `SpellType::Charges` (no Scrolls)

### 4. Verificación: `CheckPlrSpell()` (Source/player.cpp:3213-3221)

Usa el cooldown efectivo para decidir si el hechizo está disponible:

```cpp
if (IsAnyOf(spellType, SpellType::Spell, SpellType::Charges)) {
    uint16_t cd = GetEffectiveCooldown(spellID, myPlayer.GetSpellLevel(spellID));
    if (cd > 0) {
        int idx = static_cast<int>(spellID);
        if (myPlayer.spellCooldownStart[idx] != 0
            && (gGameTicks - myPlayer.spellCooldownStart[idx]) < cd) {
            return;  // bloqueado: aún en cooldown
        }
    }
}
```

Además, `myPlayer._pmode == PM_SPELL` previene double-cast durante la animación.

### 5. Expiración

La limpieza ocurre en dos lugares:
- **HUD** (`cooldown_hud.cpp`): cuando `elapsed >= effectiveCd`, se setea `spellCooldownStart[i] = 0`
- **Cambio de nivel** (`RemovePlrMissiles()` en `Source/player.cpp:2914`): `memset(MyPlayer->spellCooldownStart, 0, ...)`

## Valores por hechizo

A velocidad Normal (20 ticks/s), 20 ticks = 1 segundo. Los valores CDR (Cooldown Reduction) son el % que se reduce por cada nivel de hechizo.

| Hechizo | Ticks | CDR% | Lv1 | Lv2 | Lv3 | Lv4 |
|---|---|---|---|---|---|---|
| Firebolt | 20 | 20 | 1.00s | 0.80s | 0.60s | 0.40s |
| ChargedBolt | 30 | 20 | 1.50s | 1.20s | 0.90s | 0.60s |
| HolyBolt | 30 | 20 | 1.50s | 1.20s | 0.90s | 0.60s |
| Healing | 60 | 10 | 3.00s | 2.70s | 2.40s | 2.10s |
| HealOther | 60 | 10 | 3.00s | 2.70s | 2.40s | 2.10s |
| Lightning | 40 | 15 | 2.00s | 1.70s | 1.40s | 1.10s |
| Flash | 60 | 15 | 3.00s | 2.55s | 2.10s | 1.65s |
| Inferno | 40 | 15 | 2.00s | 1.70s | 1.40s | 1.10s |
| FireWall | 80 | 15 | 4.00s | 3.40s | 2.80s | 2.20s |
| LightningWall | 80 | 15 | 4.00s | 3.40s | 2.80s | 2.20s |
| Fireball | 40 | 15 | 2.00s | 1.70s | 1.40s | 1.10s |
| ChainLightning | 80 | 15 | 4.00s | 3.40s | 2.80s | 2.20s |
| FlameWave | 80 | 15 | 4.00s | 3.40s | 2.80s | 2.20s |
| Guardian | 160 | 15 | 8.00s | 6.80s | 5.60s | 4.40s |
| Immolation | 160 | 15 | 8.00s | 6.80s | 5.60s | 4.40s |
| Phasing | 60 | 15 | 3.00s | 2.55s | 2.10s | 1.65s |
| Telekinesis | 100 | 15 | 5.00s | 4.25s | 3.50s | 2.75s |
| BloodStar | 60 | 15 | 3.00s | 2.55s | 2.10s | 1.65s |
| Warp | 60 | 15 | 3.00s | 2.55s | 2.10s | 1.65s |
| Elemental | 100 | 15 | 5.00s | 4.25s | 3.50s | 2.75s |
| Jester | 60 | 15 | 3.00s | 2.55s | 2.10s | 1.65s |
| RingOfFire | 120 | 15 | 6.00s | 5.10s | 4.20s | 3.30s |
| TownPortal | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| StoneCurse | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| ManaShield | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| Nova | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| Teleport | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| Apocalypse | 300 | 10 | 15.00s | 13.50s | 12.00s | 10.50s |
| BoneSpirit | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| Reflect | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| Berserk | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| RuneOfFire | 60 | 10 | 3.00s | 2.70s | 2.40s | 2.10s |
| RuneOfLight | 60 | 10 | 3.00s | 2.70s | 2.40s | 2.10s |
| RuneOfNova | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| RuneOfImmolation | 160 | 10 | 8.00s | 7.20s | 6.40s | 5.60s |
| RuneOfStone | 100 | 10 | 5.00s | 4.50s | 4.00s | 3.50s |
| Raise Skeleton | 1200 | 25 | 60.00s | 45.00s | 30.00s | 15.00s |
| Golem | 200 | 5 | 10.00s | 9.50s | 9.00s | 8.50s |

Hechizos sin cooldown (`sCooldown = 0`): Null, Identify, Infravision, Resurrect, Etherealize, ItemRepair, StaffRecharge, TrapDisarm, DoomSerpents, BloodRitual, Invisibility, Rage, Mana, Magi, Search.

## HUD visual

El HUD se renderiza en `Source/qol/cooldown_hud.cpp` → `DrawCooldownHud()`, llamado desde `Source/engine/render/scrollrt.cpp` después de `DrawMinionStatus()`.

### Diseño

- Fila de cajas centradas sobre el panel, cada una con icono del hechizo + cuenta regresiva
- Solo se muestra en dungeon (`leveltype != DTYPE_TOWN`)
- Solo se muestran cooldowns activos
- Cuenta regresiva en formato `X.X` (1 decimal)

### Cálculo del tiempo restante

Usa `GetEffectiveCooldown()` para obtener la duración según el nivel del hechizo:

```cpp
uint16_t cd = GetEffectiveCooldown(id, myPlayer.GetSpellLevel(id));
uint32_t elapsed = now - start;
float ticksPerSecond = 1000.0f / static_cast<float>(gnTickDelay);
entries[i].remaining = static_cast<float>(cd - elapsed) / ticksPerSecond;
```

El valor mostrado se adapta a la velocidad actual del juego y al cooldown efectivo.

### Layout

```
BoxWidth = IconSize(37) + 2 * BoxPadding(2) = 41px
Gap = 4px entre cajas
Posición: centrado horizontal sobre el panel, 4px arriba del panel
```

## Comportamiento con pausa

El cooldown se congela automáticamente porque `gGameTicks` no avanza cuando el juego está pausado:

| Evento | Qué pasa | gGameTicks | Cooldown |
|---|---|---|---|
| Escape (SP) | `diablo_pause_game()` setea `PauseMode = 2` | Congelado | Congelado |
| Pérdida de foco | `diablo_focus_pause()` setea `PauseMode = 2` | Congelado | Congelado |
| Restaurar foco | `diablo_focus_unpause()` setea `PauseMode = 0` | Reanuda | Reanuda |

## Comportamiento con velocidad del juego

El cooldown escala con la velocidad porque está en ticks:

- A **Normal** (20 ticks/s): Firebolt (20 ticks) = 1.0s real
- A **Fastest** (50 ticks/s): Firebolt (20 ticks) = 0.4s real

## Cooldown Reduction por spell level

Cada hechizo tiene un porcentaje de reducción (`sCooldownReduction`) que se aplica por nivel de hechizo:

```
effectiveCd = baseCd * (100 - sCooldownReduction * (spellLevel - 1)) / 100
```

Ejemplo Firebolt (20 ticks, 20% CDR):

| Spell Level | Cálculo | Cooldown | Tiempo (Normal) |
|---|---|---|---|
| 1 | 20 × 100% | 20 ticks | 1.00s |
| 2 | 20 × 80% | 16 ticks | 0.80s |
| 3 | 20 × 60% | 12 ticks | 0.60s |
| 4 | 20 × 40% | 8 ticks | 0.40s |

## Multiplayer

1. `diablo_pause_game()` tiene `if (!gbIsMultiplayer)` — Escape no pausa en multiplayer
2. Los cooldowns son estado local (`MyPlayer->spellCooldownStart[]`), no se sincronizan por red
3. `diablo_focus_pause()` sí pausa al perder foco localmente (el juego sigue corriendo para los demás)

## Balance de Firebolt

### Fórmula de daño (`Source/missiles.cpp:1810`)

```cpp
missile._midam = GenerateRnd(10) + (player._pMagic / 8) + missile._mispllvl * 2 + 1;
```

El multiplicador `*2` compensa la reducción de `MaxSpellLevel` de 15 (vanilla) a 4 (mod). Sin este cambio, Firebolt perdería ~11 puntos de daño base a nivel máximo.

### Daño por nivel de hechizo

| _pMagic | Lv1 | Lv2 | Lv3 | Lv4 |
|---|---|---|---|---|
| 30 | 5-14 | 7-16 | 9-18 | 11-20 |
| 50 | 8-17 | 10-19 | 12-21 | 14-23 |
| 80 | 11-20 | 13-22 | 15-24 | 17-26 |
| 120 | 16-25 | 18-27 | 20-29 | 22-31 |
| 150 | 20-29 | 22-31 | 24-33 | 26-35 |

### Nivel inicial

El hechicero empieza con Firebolt nivel 1 (`Source/player.cpp:2344`), no nivel 2 como en vanilla.

## Minion Status HUD

### Posicionamiento (`Source/qol/minionstatus.cpp`)

- **Sin automap:** `x = 8`, `y = 4` (arriba a la izquierda)
- **Con automap activo:** las cajas se desplazan hacia abajo para no solaparse con el texto del automap:
  - Single-player: `y = 42` (debajo de "Level: N" + "Difficulty: ...")
  - Multiplayer: `y = 72` (debajo de las 4 líneas de texto del automap)

### Apilado

Las cajas se apilan **verticalmente** (Skeleton arriba, Golem debajo), con `gap = 4px` entre ellas.

`x += BoxWidth + gap` → `y += BoxHeight + gap`

## Notas

### Debug God Mode

En builds de debug, `DebugGodMode` fuerza `hitPoints = 0` en cualquier monstruo golpeado por un misil (`MonsterTrapHit` en `missiles.cpp:968-970`). Esto incluye minions propios (Skeleton/Golem), causando muerte instantánea. No afecta builds de release.

## Archivos modificados

| Archivo | Cambio |
|---|---|
| `Source/diablo.h` | `extern uint32_t gGameTicks;` (reemplaza `gPausedTime` + `GetGameTime()`) |
| `Source/diablo.cpp` | `gGameTicks = 0`, se incrementa en `GameLogic()` después de `ProcessInput()` |
| `Source/diablo.cpp` | `diablo_pause_game()`, `diablo_focus_pause()`, `diablo_focus_unpause()` simplificados |
| `Source/spelldat.h` | `uint16_t sCooldown` + `uint8_t sCooldownReduction` en `SpellData` |
| `Source/spelldat.h` | `GetEffectiveCooldown(spellId, spellLevel)` — cooldown efectivo con reducción |
| `Source/spelldat.cpp` | Valores de `sCooldown` + `sCooldownReduction` para los 53 hechizos |
| `Source/player.h` | `uint32_t spellCooldownStart[64]` en struct `Player` |
| `Source/player.cpp` | Registro en `DoSpell()`, verificación con `GetEffectiveCooldown()` en `CheckPlrSpell()` |
| `Source/player.cpp` | Reset en `RemovePlrMissiles()`, Hechicero Firebolt lvl 1 |
| `Source/player.cpp` | `myPlayer._pmode == PM_SPELL` previene double-cast |
| `Source/missiles.cpp` | Firebolt: `_mispllvl * 2` en fórmula de daño |
| `Source/qol/cooldown_hud.cpp` | `DrawCooldownHud()` — HUD con cooldown efectivo |
| `Source/qol/cooldown_hud.h` | Declaración de `DrawCooldownHud()` |
| `Source/CMakeLists.txt` | `qol/cooldown_hud.cpp` agregado al build |
| `Source/engine/render/scrollrt.cpp` | Llamada a `DrawCooldownHud()` después de `DrawMinionStatus()` |
| `Source/qol/minionstatus.cpp` | Reposicionado arriba a la izquierda, apilado vertical, baja con automap |
| `Source/panels/spell_list.cpp` | Overlay de cooldown removido del spell panel |
