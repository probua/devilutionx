# Etapa 7: Pulido — GetVirtualLevel(), diálogos, tiendas, daño

## Objetivo

Crear un sistema de mapeo de niveles virtual unificado (`GetVirtualLevel()`) y aplicarlo a todos los sistemas de gameplay que escalan con el nivel: daño de trampas, santuarios, desarmado, tiendas, y diálogos de voz. También corregir los checks hardcoded restantes.

## El problema fundamental

El juego original escala muchas cosas con `currlevel` (1-16): daño de trampas, dificultad de desarmado, experiencia de santuarios, calidad de tiendas, etc. Con el mod usando niveles 1-7, estos valores quedaban reducidos a la mitad o menos.

**Ejemplo:** Una trampa de flechas en nivel 16 (Diablo original) hacía `currlevel * 2 = 32` de daño máximo. En el mod nivel 7, seria `7 * 2 = 14` — un 56% menos.

## Funciones helper

### `GetVirtualLevel()` — `Source/levels/gendung.h`

```cpp
inline int GetVirtualLevel()
{
    static constexpr int VirtualLevelMap[] = { 0, 1, 3, 5, 7, 9, 13, 16 };
    if (currlevel >= 1 && currlevel <= 7)
        return VirtualLevelMap[currlevel];
    return currlevel;
}
```

Mapea `currlevel` real al nivel equivalente del sistema original. Para niveles > 7 (Hellfire), devuelve `currlevel` sin modificar.

### `GetVirtualLevelForShop(int level)` — `Source/levels/gendung.h`

Igual que `GetVirtualLevel()` pero acepta un nivel arbitrario (no usa la global `currlevel`). Usado por tiendas y `ItemsGetCurrlevel()` para set levels.

## Archivos modificados

### `Source/items.cpp` — Refactorización

`ItemsGetCurrlevel()` fue refactorizado para usar los helpers en vez de su propio `LevelMap` duplicado:

- Rama normal: `return GetVirtualLevel()`
- Rama set level: `return GetVirtualLevelForShop(qlevel)`

Elimina duplicación de datos — un solo array de mapeo en `gendung.h`.

### `Source/missiles.cpp` — Daño de misiles y trampas

#### Fórmulas aritméticas (15 cambios)

Todas las fórmulas de daño que usaban `currlevel` directamente ahora usan `GetVirtualLevel()`:

| Función/Línea | Fórmula original | Fórmula nueva | Efecto |
|---|---|---|---|
| `ProjectileTrapDamage()` | `currlevel + rnd(2*currlevel)` | `GetVirtualLevel() + rnd(2*GetVirtualLevel())` | Daño base trampas |
| `AddFlashBottom()` trap | `currlevel / 2` | `GetVirtualLevel() / 2` | Flash spell trampa |
| `AddFlashTop()` trap | `currlevel / 2` | `GetVirtualLevel() / 2` | Flash spell trampa |
| `AddManaOrLifeSteal()` trap | `(currlevel/2) + rndsum` | `(GetVirtualLevel()/2) + rndsum` | Bone Spirit trampa |
| Arrow trap (físico) | `rnd(10)+1+currlevel` | `rnd(10)+1+GetVirtualLevel()` | Daño min flechas |
| Arrow trap (físico) | `rnd(10)+1+currlevel*2` | `rnd(10)+1+GetVirtualLevel()*2` | Daño max flechas |
| Arrow trap (rayo) | `rnd(10)+1+currlevel` | `rnd(10)+1+GetVirtualLevel()` | Daño min rayo |
| Arrow trap (rayo) | `rnd(10)+1+currlevel*2` | `rnd(10)+1+GetVirtualLevel()*2` | Daño max rayo |
| Arrow trap (fuego) | `rnd(10)+1+currlevel` | `rnd(10)+1+GetVirtualLevel()` | Daño min fuego |
| Arrow trap (fuego) | `rnd(10)+1+currlevel*2` | `rnd(10)+1+GetVirtualLevel()*2` | Daño max fuego |
| `ProcessArrow()` trap | `currlevel` | `GetVirtualLevel()` | Daño min genérico |
| `ProcessArrow()` trap | `2 * currlevel` | `2 * GetVirtualLevel()` | Daño max genérico |
| `ProcessLightningControl()` | `rnd(currlevel) + 2*currlevel` | `rnd(GetVirtualLevel()) + 2*GetVirtualLevel()` | Daño lightning |
| `AddFireWall()` trap | `currlevel` | `GetVirtualLevel()` | Daño Fire Wall |
| `AddFireWall()` range | `currlevel` | `GetVirtualLevel()` | Duración Fire Wall |

#### Minhit — Hit mínimo de monstruos (3 checks)

```cpp
// Original:
if (currlevel == 14) minhit = 20;
if (currlevel == 15) minhit = 25;
if (currlevel == 16) minhit = 30;

// Mod:
if (currlevel == 6)  minhit = 20;
if (currlevel == 7)  minhit = 30;
```

Se eliminó el nivel intermedio (15→25) porque Hell solo tiene 2 niveles. Nivel 6 = 20, nivel 7 = 30.

### `Source/objects.cpp` — Santuarios

| Santuario | Original | Nuevo | Efecto |
|---|---|---|---|
| Oily (Fire Wall) | `2 * currlevel + 2` | `2 * GetVirtualLevel() + 2` | Daño del Fire Wall del santuario |
| Sparkling (XP) | `1000 * currlevel` | `1000 * GetVirtualLevel()` | XP otorgada: 7000 en nivel 7 vs 700 original |
| Sparkling (Flash) | `3 * currlevel + 2` | `3 * GetVirtualLevel() + 2` | Daño del Flash del santuario |

### `Source/player.cpp` — Diálogos de voz

La función `PlayDungMsgs()` reproduce una frase del héroe la primera vez que entra a una zona nueva. Usa un flag (`pDungMsgs`) para no repetirse.

**Cambios:**

| Zona | Original | Mod | `_pLvlVisited` | Diálogo |
|---|---|---|---|---|
| Cathedral | `currlevel == 1` | `1` (sin cambio) | `[1]` | "The sanctity of this place has been fouled" |
| Catacombs | `currlevel == 5` | `3` | `[3]` | "The smell of death surrounds me" |
| Caves | `currlevel == 9` | `5` | `[5]` | "It's hot down here" |
| Hell | `currlevel == 13` | `6` | `[6]` | "I must be getting close" |
| Diablo | `currlevel == 16` | `7` | `[7]` | Sonido PS_DIABLVLINT |

Los indices de `_pLvlVisited[]` se cambiaron para coincidir con los nuevos números de nivel.

Las líneas Hellfire (17, 19, 21) no se tocaron.

### `Source/player.cpp` — Desarmar trampas

```cpp
// Original:
int trapdisper = 2 * player._pDexterity - 5 * currlevel;

// Mod:
int trapdisper = 2 * player._pDexterity - 5 * GetVirtualLevel();
```

Esto mantiene la dificultad de desarmado proporcional al nivel virtual. En nivel 7 (Diablo), la penalización es `-5 * 16 = -80`, igual que el original.

### `Source/stores.cpp` — Inventario de tiendas

La función `SetupTownStores()` determina la calidad de items que venden los NPCs.

**Original:**
```cpp
l = clamp(l + 2, 6, 16);  // Shop level entre 6 y 16
```

En SP, `l` se calculaba como el nivel más alto visitado (`_pLvlVisited[i]`). Con niveles 1-7, el máximo seria 7, dando `clamp(7+2, 6, 16) = 9`. Las tiendas venderian items de calidad nivel 9 (Caves), perdiendo todos los items de Hell (10-16).

**Cambio:**
```cpp
l = clamp(l + 1, 1, 7);       // Primero: asegurar rango 1-7
l = GetVirtualLevelForShop(l); // Mapear a nivel virtual (1-16)
l = clamp(l, 6, 16);          // Final: clamp al rango de tiendas
```

Ahora al visitar nivel 7: `l = clamp(7+1, 1, 7) = 7` → `GetVirtualLevelForShop(7) = 16` → `clamp(16, 6, 16) = 16`. Las tiendas venden items de calidad máxima, igual que el original.

Nota: `SpawnBoy()` y `SpawnPremium()` usan `_pLevel` (nivel del personaje), no el nivel de mazmorra, así que no necesitan cambio.

## Sistemas que NO necesitan cambio

| Sistema | Archivo | Razón |
|---|---|---|
| Level transitions | `interfac.cpp` | `currlevel--` es genérico |
| Trigger labels | `trigs.cpp` | `currlevel ± 1` funciona con 1-7 |
| Automap display | `automap.cpp` | Solo Hellfire (17-24) |
| MP messages | `msg.cpp` | Solo check Hellfire (lvl 24) |
| `glSeedTbl[0]` | `msg.cpp`, `loadsave.cpp` | Es town (nivel 0) |
| SpawnBoy/SpawnPremium | `items.cpp` | Usan `_pLevel`, no nivel de mazmorra |

## Ventajas del enfoque GetVirtualLevel()

1. **Un solo punto de cambio** — Si queremos ajustar el mapeo (ej: nivel 7 → 14 en vez de 16), cambiamos un solo array
2. **Consistencia** — Todos los sistemas ven el mismo nivel virtual
3. **Sin duplicación** — `ItemsGetCurrlevel()` se simplifica usando el helper
4. **Hellfire seguro** — Niveles > 7 devuelven `currlevel` sin modificar
5. **Mantenible** — Agregar nuevos sistemas que escalen con nivel es trivial: usar `GetVirtualLevel()`

## Bug fix: Cap anti-power-leveling en multiplayer

### Problema

El `ExperienceMultiplier` (x10) se aplicaba antes del cap anti-power-leveling de multiplayer. El cap limita la XP por kill al mínimo de:

1. La XP calculada (con multiplier)
2. `ExpLvlsTbl[nivel] / 20` — 1/20 de la XP necesaria para el nivel actual
3. `200 * nivel` — 200 veces el nivel del personaje

Con el multiplier aplicado primero, el cap cortaba la XP multiplicada. Resultado: en multiplayer la XP era mayor que vanilla pero **no x10**.

Ejemplo: personaje nivel 5, monstruo da 100 XP base:
- **Sin multiplier:** cap = min(100, ExpLvlsTbl[5]/20, 1000) = 100 ✓
- **Con multiplier ANTES del cap:** clampedExp = 1000, cap = min(1000, 250, 1000) = 250 ✗
- **Con multiplier DESPUÉS del cap:** cap = min(100, 250, 1000) = 100, luego x10 = 1000 ✓

### Fix

Se reestructuró `AddPlrExperience()` en `player.cpp`:

```cpp
// ANTES (roto): multiplier → cap → aplicar
clampedExp = exp * 10 * levelScaling;  // multiplier
if (gbIsMultiplayer)
    clampedExp = min(clampedExp, cap);  // cap corta el multiplier

// DESPUÉS (correcto): cap sobre base → multiplier → aplicar
if (gbIsMultiplayer)
    exp = min(exp, cap);               // cap sobre XP base
clampedExp = exp * 10 * levelScaling;  // multiplier aplica después
```

El cap anti-power-leveling ahora se aplica sobre la XP base (sin multiplier), preservando la intención original del límite (prevenir power-leveling con XP base excesiva) mientras el multiplier x10 funciona correctamente en ambos modos.
