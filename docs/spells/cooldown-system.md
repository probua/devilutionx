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

### 1. Datos: `sCooldown` en SpellData

Cada hechizo tiene un campo `uint16_t sCooldown` en `Source/spelldat.h:235` que define su duración en **game ticks** (no milisegundos). Valor `0` = sin cooldown.

### 2. Registro: `DoSpell()` (Source/player.cpp:1071-1076)

El cooldown se registra **cuando el hechizo efectivamente se ejecuta** (en `DoSpell()`), no cuando el jugador hace click (en `CheckPlrSpell()`). Esto previene cooldowns falsos si la animación se cancela.

```cpp
if (IsAnyOf(player.executedSpell.spellType, SpellType::Spell, SpellType::Charges)) {
    uint16_t cd = GetSpellData(player.executedSpell.spellId).sCooldown;
    if (cd > 0) {
        player.spellCooldownStart[static_cast<int>(player.executedSpell.spellId)] = gGameTicks;
    }
}
```

- `spellCooldownStart[64]` en `Player` (`Source/player.h:342`) almacena el game tick en que se registró el cooldown para cada `SpellID`
- Solo se registran cooldowns para `SpellType::Spell` y `SpellType::Charges` (no Scrolls)

### 3. Verificación: `CheckPlrSpell()` (Source/player.cpp:3213-3221)

Antes de permitir un cast, se verifica si el cooldown ya expiró:

```cpp
if (IsAnyOf(spellType, SpellType::Spell, SpellType::Charges)) {
    uint16_t cd = GetSpellData(spellID).sCooldown;
    if (cd > 0) {
        int idx = static_cast<int>(spellID);
        if (myPlayer.spellCooldownStart[idx] != 0
            && (gGameTicks - myPlayer.spellCooldownStart[idx]) < cd) {
            return;  // bloqueado: aún en cooldown
        }
    }
}
```

Además, un chequeo de `myPlayer._pmode == PM_SPELL` previene double-cast durante la animación.

### 4. Expiración

El cooldown expira naturalmente cuando `gGameTicks - spellCooldownStart[i] >= sCooldown`. La limpieza ocurre en dos lugares:

- **HUD** (`cooldown_hud.cpp:55-58`): Si `elapsed >= cd`, se setea `spellCooldownStart[i] = 0` y no se muestra
- **Cambio de nivel** (`RemovePlrMissiles()` en `Source/player.cpp:2914`): `memset(MyPlayer->spellCooldownStart, 0, ...)` limpia todos los cooldowns

## Valores por hechizo

Todos los valores están en **game ticks**. A velocidad Normal (20 ticks/s), 20 ticks = 1 segundo real.

| Hechizo | Ticks | Segundos (Normal) | Segundos (Fastest) |
|---|---|---|---|
| Firebolt | 20 | 1.0 | 0.4 |
| ChargedBolt | 30 | 1.5 | 0.6 |
| HolyBolt | 30 | 1.5 | 0.6 |
| Healing | 60 | 3.0 | 1.2 |
| Lightning | 40 | 2.0 | 0.8 |
| Flash | 60 | 3.0 | 1.2 |
| Inferno | 40 | 2.0 | 0.8 |
| FireWall | 80 | 4.0 | 1.6 |
| TownPortal | 100 | 5.0 | 2.0 |
| StoneCurse | 100 | 5.0 | 2.0 |
| Phasing | 60 | 3.0 | 1.2 |
| ManaShield | 100 | 5.0 | 2.0 |
| Fireball | 40 | 2.0 | 0.8 |
| ChainLightning | 80 | 4.0 | 1.6 |
| FlameWave | 80 | 4.0 | 1.6 |
| Guardian | 160 | 8.0 | 3.2 |
| Golem | 200 | 10.0 | 4.0 |
| Nova | 100 | 5.0 | 2.0 |
| Teleport | 100 | 5.0 | 2.0 |
| Apocalypse | 300 | 15.0 | 6.0 |
| Elemental | 100 | 5.0 | 2.0 |
| Telekinesis | 100 | 5.0 | 2.0 |
| HealOther | 60 | 3.0 | 1.2 |
| BloodStar | 60 | 3.0 | 1.2 |
| BoneSpirit | 100 | 5.0 | 2.0 |
| Raise Skeleton | 160 | 8.0 | 3.2 |
| Jester | 60 | 3.0 | 1.2 |
| LightningWall | 80 | 4.0 | 1.6 |
| Immolation | 160 | 8.0 | 3.2 |
| Warp | 60 | 3.0 | 1.2 |
| Reflect | 100 | 5.0 | 2.0 |
| Berserk | 100 | 5.0 | 2.0 |
| RingOfFire | 120 | 6.0 | 2.4 |
| RuneOfFire | 60 | 3.0 | 1.2 |
| RuneOfLight | 60 | 3.0 | 1.2 |
| RuneOfNova | 100 | 5.0 | 2.0 |
| RuneOfImmolation | 160 | 8.0 | 3.2 |
| RuneOfStone | 100 | 5.0 | 2.0 |

Hechizos sin cooldown (`sCooldown = 0`): Null, Identify, Infravision, Resurrect, Etherealize, ItemRepair, StaffRecharge, TrapDisarm, DoomSerpents, BloodRitual, Invisibility, Rage, Mana, Magi, Search.

## HUD visual

El HUD se renderiza en `Source/qol/cooldown_hud.cpp` → `DrawCooldownHud()`, llamado desde `Source/engine/render/scrollrt.cpp` después de `DrawMinionStatus()`.

### Diseño

- Fila de cajas centradas sobre el panel, cada una con icono del hechizo + cuenta regresiva
- Solo se muestra en dungeon (`leveltype != DTYPE_TOWN`)
- Solo se muestran cooldowns activos
- Cuenta regresiva en formato `X.X` (1 decimal)

### Cálculo del tiempo restante

```cpp
float ticksPerSecond = 1000.0f / static_cast<float>(gnTickDelay);
entries[i].remaining = static_cast<float>(cd - elapsed) / ticksPerSecond;
```

El valor mostrado se adapta a la velocidad actual del juego: si la velocidad cambia, el display se actualiza correctamente.

### Layout

```
BoxWidth = IconSize(37) + 2 * BoxPadding(2) = 41px
Gap = 4px entre cajas
Posición: centrado horizontal sobre el panel, 4px arriba del panel
```

## Comportamiento con pausa

El cooldown se congela automáticamente cuando el juego se pausa porque `gGameTicks` no avanza:

| Evento | Qué pasa | gGameTicks | Cooldown |
|---|---|---|---|
| Escape (SP) | `diablo_pause_game()` setea `PauseMode = 2` | Congelado | Congelado |
| Pérdida de foco | `diablo_focus_pause()` setea `PauseMode = 2` | Congelado | Congelado |
| Restaurar foco | `diablo_focus_unpause()` setea `PauseMode = 0` | Reanuda | Reanuda |

No hay acumulación manual de tiempo pausado ni variables auxiliares — el contador simplemente deja de avanzar.

## Comportamiento con velocidad del juego

El cooldown escala naturalmente con la velocidad porque está en ticks:

- A **Normal** (20 ticks/s): Firebolt (20 ticks) = 1.0s real
- A **Fastest** (50 ticks/s): Firebolt (20 ticks) = 0.4s real

Esto es consistente con cómo el resto del juego mide el tiempo: monstruos, animaciones, misiles — todo avanza más rápido con mayor `nTickRate`.

## Multiplayer

**No requiere consideración especial.** Razones:

1. `diablo_pause_game()` tiene `if (!gbIsMultiplayer)` — Escape no pausa en multiplayer
2. Los cooldowns son estado local (`MyPlayer->spellCooldownStart[]`), no se sincronizan por red
3. `diablo_focus_pause()` sí pausa al perder foco, pero es correcto: si el jugador no ve la pantalla, sus cooldowns no deberían avanzar localmente (el juego sigue corriendo para los demás)

## Archivos modificados

| Archivo | Cambio |
|---|---|
| `Source/diablo.h` | `extern uint32_t gGameTicks;` (reemplaza `gPausedTime` + `GetGameTime()`) |
| `Source/diablo.cpp` | `gGameTicks = 0`, se incrementa en `GameLogic()` después de `ProcessInput()` |
| `Source/diablo.cpp` | `diablo_pause_game()`, `diablo_focus_pause()`, `diablo_focus_unpause()` simplificados (sin tracking de tiempo) |
| `Source/spelldat.h` | Campo `uint16_t sCooldown` en `SpellData` |
| `Source/spelldat.cpp` | Valores de `sCooldown` para los 53 hechizos (en ticks) |
| `Source/player.h` | `uint32_t spellCooldownStart[64]` en struct `Player` |
| `Source/player.cpp` | Registro en `DoSpell()`, verificación en `CheckPlrSpell()`, reset en `RemovePlrMissiles()` |
| `Source/qol/cooldown_hud.cpp` | `DrawCooldownHud()` — renderizado del HUD |
| `Source/qol/cooldown_hud.h` | Declaración de `DrawCooldownHud()` |
| `Source/CMakeLists.txt` | `qol/cooldown_hud.cpp` agregado al build |
| `Source/engine/render/scrollrt.cpp` | Llamada a `DrawCooldownHud()` después de `DrawMinionStatus()` |
| `Source/qol/minionstatus.cpp` | Movido a top-center de pantalla |
| `Source/panels/spell_list.cpp` | Overlay de cooldown removido del spell panel |
