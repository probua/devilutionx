# Mod Diablo 7 Niveles — Overview

## Objetivo

Reducir la mazmorra de Diablo 1 de 16 niveles a 7, manteniendo la progresión de items, monstruos y dificultad equivalente al juego original. Además: multiplicar XP por 10 y compartir XP entre todos los jugadores.

## Distribución de niveles

| Nivel mod | Tema | Nivel original equivalente |
|---|---|---|
| 1 | Cathedral | 1 |
| 2 | Cathedral | 3 |
| 3 | Catacombs | 5 |
| 4 | Catacombs | 7 |
| 5 | Caves | 9 |
| 6 | Hell | 13 |
| 7 | Hell (Diablo) | 16 |

**Distribución original:** 4 Cathedral + 4 Catacombs + 4 Caves + 4 Hell = 16 niveles  
**Distribución mod:** 2 Cathedral + 2 Catacombs + 1 Caves + 2 Hell = 7 niveles

## Mapeo virtual de niveles

El cambio más importante del mod es el sistema de **mapeo virtual**. Todos los sistemas del juego (items, daños, trampas, santuarios, tiendas) que escalaban con `currlevel` (1-16) ahora reciben un nivel "virtual" mapeado:

```
LevelMap = { 0, 1, 3, 5, 7, 9, 13, 16 }
```

| currlevel real | GetVirtualLevel() | Significado |
|---|---|---|
| 1 | 1 | Cathedral temprana |
| 2 | 3 | Cathedral tardía |
| 3 | 5 | Catacombs temprana |
| 4 | 7 | Catacombs tardía |
| 5 | 9 | Caves |
| 6 | 13 | Hell temprana |
| 7 | 16 | Hell tardía / Diablo |

## Funciones helper

Definidas en `Source/levels/gendung.h`:

- **`GetVirtualLevel()`** — Mapea `currlevel` (1-7) al rango original (1-16). Usado por daño de trampas, santuarios, desarmar trampas, Fire Wall, etc.
- **`GetVirtualLevelForShop(int level)`** — Mapea un nivel arbitrario al rango original. Usado por tiendas y `ItemsGetCurrlevel()`.

## Constantes clave

| Constante | Valor | Nota |
|---|---|---|
| `NUMLEVELS` | 25 | Sin cambio (seguridad para buffers de Hellfire) |
| `giNumberOfLevels` | 8 | Era 17 (Diablo) o 25 (Hellfire). 7 niveles + town = 8 |
| `ExperienceMultiplier` | 10 | En `player.cpp`, multiplica toda la XP ganada |

## Quests activas (7)

| Quest | Nivel mod | Nivel original | Tipo |
|---|---|---|---|
| Butcher (Q_BUTCHER) | 1 | 2 | Cathedral |
| Skeleton King (Q_SKELKING) | 2 | 1 | Cathedral (set level, SP y MP) |
| Halls of the Blind (Q_BLIND) | 3 | 7 | Catacombs |
| Valor (Q_BLOOD) | 4 | 8 | Catacombs |
| Anvil of Fury (Q_ANVIL) | 5 | 9 | Caves |
| Betrayer/Lazarus (Q_BETRAYER) | 6 | 15 | Hell (set level) |
| Diablo (Q_DIABLO) | 7 | 16 | Hell |

## Quests eliminadas (9)

Q_ROCK, Q_MUSHROOM, Q_GARBUD, Q_ZHAR, Q_VEIL, Q_LTBANNER, Q_PWATER, Q_WARLORD, Q_SCHAMB

Eliminadas via `QUEST_NOTAVAIL` forzado en `InitQuests()`.

## Etapas del desarrollo

1. **Niveles y temas** — `GetLevelType()`, `giNumberOfLevels`
2. **Warps y triggers** — Escaleras, portales, transiciones entre niveles
3. **Monstruos** — Remapeo de 93 monstruos en `monstdat.cpp`
4. **Quests** — 7 activas, 9 eliminadas, pools simplificados
5. **Items y objetos** — `ItemsGetCurrlevel()`, story books, objetos de Diablo
6. **Generación de mazmorras** — `drlg_l*.cpp`, `themes.cpp`
7. **Pulido** — `GetVirtualLevel()`, diálogos de voz, tiendas, daño de trampas

## Archivos modificados

| Archivo | Etapa(s) | Cambio principal |
|---|---|---|
| `Source/levels/gendung.h` | 7 | `GetVirtualLevel()`, `GetVirtualLevelForShop()` |
| `Source/levels/gendung.cpp` | 1 | `GetLevelType()` remapeado |
| `Source/diablo.cpp` | 1 | `giNumberOfLevels = 8` |
| `Source/levels/trigs.cpp` | 2 | Warps, escaleras, triggers |
| `Source/levels/town.cpp` | 2 | Spawn positions para warps |
| `Source/interfac.cpp` | 2 | Cutscenes (nivel 7 = Diablo) |
| `Source/monstdat.cpp` | 3 | 93 monstruos remapeados |
| `Source/monster.cpp` | 3 | `GetLevelMTypes()`, `InitMonsters()` |
| `Source/quests.cpp` | 4 | `QuestsData[]`, `InitQuests()`, pools |
| `Source/towners.cpp` | 4 | `_pLvlVisited` dinámico |
| `Source/objects.cpp` | 4, 5 | Quest objects, story books, Diablo levers |
| `Source/control.cpp` | 4 | `glSeedTbl[7]` |
| `Source/items.cpp` | 5 | `ItemsGetCurrlevel()` con mapeo virtual |
| `Source/levels/drlg_l2.cpp` | 6 | Quest room sizing, town warp |
| `Source/levels/drlg_l3.cpp` | 6 | Town warp stairs |
| `Source/levels/drlg_l4.cpp` | 6 | Lazarus, Diablo, PlaceStairs reestructurado |
| `Source/levels/themes.cpp` | 6 | Theme rooms exclusion nivel 7 |
| `Source/missiles.cpp` | 7 | Daño trampas, minhit, Fire Wall |
| `Source/player.cpp` | 7 | XP x10, shared XP, voice dialogs, trap disarm |
| `Source/stores.cpp` | 7 | Shop level mapping |
| `Source/spelldat.h` / `.cpp` | Spells | `Skeleton=37`, spell tier data, `MAX_SPELLS=53` |
| `Source/panels/spell_book.cpp` | Spells | Layout páginas 0-1 (Skeleton, Telekinesis, Null, Inferno) |
| `Source/panels/spell_icons.cpp` | Spells | `SpellITbl[37]=24`, array 53 entradas |
| `Source/misdat.cpp` | Spells | `MissilesData[Skeleton]` con AddSkeleton |
| `Source/missiles.cpp` / `.h` | Spells | `AddSkeleton()`, 15 damage formulas con `GetVirtualLevel()` |
| `Source/monster.cpp` / `.h` | Spells + Minions | skeletonTypeIndex, KillMySkeleton, SetMapMonsters slot reservation, golem idle freeze |
| `Source/minion_ai.cpp` / `.h` | Minions | `GolumAi` (3 estados via `goal`), `ScanForEnemy`, `MoveToward`, `ActivateNearbyMonsters`, `ClearMinionTarget`, `InitGolems`, `InitSkeletons`, `PreSpawnSkeleton`, `SpawnGolem`, `SpawnSkeleton` (movidos desde `monster.cpp`) |
| `Source/automap.cpp` | Minions | `DrawAutomapMinion()` — golem y esqueleto visibles en automapa (flecha verde) |
| `Source/qol/minionstatus.cpp` / `.h` | Minions | `DrawMinionStatus()` — HUD centrado con icono + barra HP + debug state |
| `Source/engine/render/scrollrt.cpp` | Minions | Llamada a `DrawMinionStatus` después de `DrawXPBar` |
| `Source/msg.cpp` / `.h` | Spells | CMD_AWAKESKELETON, NetSendCmdSkeleton, OnAwakeSkeleton, DeltaSyncSkeleton, DeltaLoadLevel type check |

## Decisiones de diseño

### Generales

- `NUMLEVELS` se mantiene en 25 para evitar buffer overflows en código de Hellfire
- Código de Hellfire no se elimina, solo no se activa (`gbIsHellfire == false`)
- Lazarus es un **set level** (`SL_VILEBETRAYER`) — portal desde nivel 6, no ocupa un `currlevel`
- Diablo boss se coloca via `GetLevelMTypes()` hardcoded, no via `minDunLvl`/`maxDunLvl`
- Item generation usa mapeo virtual en vez de modificar `iMinMLvl` en `itemdat.cpp`
- El nivel 6 (Hell) hace doble función: es el primer nivel de Hell (town warp) Y el nivel de Lazarus (pentagrama)

### Minions (Golem y Raise Skeleton)

Ambos minions usan la misma IA (`GolumAi`) con un sistema de estados basado en `Monster::goal`. La IA vive en `Source/minion_ai.cpp` / `Source/minion_ai.h` (separado de `monster.cpp`).

#### States (using `Monster::goal`)

| `goal` | var1 | Name | When | Behavior |
|---|---|---|---|---|
| `MonsterGoal::Normal` | 2 (Idle) | IDLE | Default state | Scans for enemies via `ScanForEnemy()`, faces owner. Transitions to FOLLOW if owner >8 tiles, or CHASE if enemy found |
| `MonsterGoal::Move` | 0 (Follow) | FOLLOW | Owner >8 tiles away | Walks toward owner with 4-tick delay. Transitions to IDLE when owner ≤6 tiles (hysteresis) |
| `MonsterGoal::Attack` | 1 (Chase) | CHASE | Enemy visible in same room | Pursues fixed target. Transitions to FOLLOW if owner >8 tiles, IDLE if target dies or >10 tiles away |

#### Transitions
```
IDLE ──(owner >8)──→ FOLLOW
IDLE ──(enemy visible)──→ CHASE
FOLLOW ──(owner ≤6, no enemy)──→ IDLE
FOLLOW ──(owner ≤6, enemy visible)──→ CHASE
CHASE ──(owner >8)──→ FOLLOW
CHASE ──(target dead or >10 tiles)──→ IDLE
```

#### Vision (ScanForEnemy)
- Only called from IDLE state (not every tick)
- Filters: `dTransVal` (same room) + `LineClearMissile` (line of sight)
- Minion cannot detect enemies through walls or doors
- Prioritizes threats to owner (≤5 tiles), then nearest enemy (≤8 tiles)
- Target is sticky — persists until lost (dead, >10 tiles, or owner >8 tiles)

#### Constants

| Constant | Value | Purpose |
|---|---|---|
| `MaxMinionReturnDistance` | 8 | Owner distance to trigger FOLLOW |
| `MinionFollowHysteresis` | 6 | Owner distance to stop FOLLOW (hysteresis) |
| `MinionChaseMaxRange` | 10 | Max distance before losing target |
| `MinionIdleDelay` | 4 | Tick delay between steps in FOLLOW |
| `MinionEngageRange` | 5 | Range for threat detection near owner |

#### Idle freeze (golem)

El golem no tiene sprite de Stand en los assets originales (0 frames). Para que no se vea "corriendo" mientras está quieto, se usa el primer frame de la animación Attack congelado con `ticksPerFrame = 127`. Solo aplica a `MT_GOLEM` — el esqueleto tiene sprites Stand propios. Al caminar/atacar, la animación se reanuda normalmente.

#### Automap

El golem y el esqueleto se muestran en el automapa (Tab) como flechas verdes:
- **Color**: verde (`MapColorsMinion = PAL16_BEIGE + 8`)
- **Solo locales**: solo los minions del jugador local
- **Detección**: se dibujan si `position.tile != GolemHoldingCell`
- Implementado en `DrawAutomapMinion()` en `Source/automap.cpp`

#### HUD de estado

Barra de estado del minion centrada horizontalmente justo arriba del panel principal:
- **Solo en dungeon** (`leveltype != DTYPE_TOWN`)
- **Solo si vivo** (`position.tile != GolemHoldingCell`)
- **Centrado**: 1 minion → centrado; 2 minions → lado a lado centrados como grupo
- **Sin minions**: no se dibuja nada

Cada cuadro (220x36px) contiene:
- **Icono del hechizo** (37x38px) — `DrawSmallSpellIcon` con el SpellID del summon
- **Nombre** — "Skeleton" o "Golem"
- **Barra de vida** (120x3px) — gradiente de 3 líneas, color según HP% (verde >60%, amarillo 30-60%, rojo <30%)
- **Texto HP** — `currHP/maxHP` (valores `>> 6` para display)

En builds debug (`_DEBUG`), se muestra un tag de estado AI a la derecha del nombre, leído directamente de `var1` vía `MinionState` enum (IDLE, FOLLOW, CHASE, ATTACK, WALK, DEAD).

Implementado en `DrawMinionStatus()` en `Source/qol/minionstatus.cpp`, llamado desde `DrawView` en `scrollrt.cpp`.

## Hotfixes post-implementación

### 2025-05-24: Storybook crash (nivel 2)

- **Bug:** Crash al entrar al nivel 2 — `Unable to find object_graphic_id 53` → Segmentation fault
- **Causa:** `InitObjectGFX()` cargaba gráficos de storybook solo para niveles originales (4, 8, 12). Al cambiar `AddStoryBooks()` a niveles 2, 4, 6, el nivel 2 no tenía los gráficos cargados.
- **Fix:** `IsAnyOf(currlevel, 4, 8, 12)` → `IsAnyOf(currlevel, 2, 4, 6)` en `objects.cpp`
- **Detalle:** Ver `mod-etapa-5-items.md` sección "Bug fix: InitObjectGFX()"

### 2025-05-24: XP multiplier en multiplayer

- **Bug:** XP x10 no funcionaba correctamente en multiplayer — el cap anti-power-leveling cortaba la XP multiplicada
- **Causa:** El cap se aplicaba después del `ExperienceMultiplier`, limitando la XP efectiva a mucho menos de x10
- **Fix:** Se reestructuró `AddPlrExperience()` para que el cap se aplique sobre la XP base (antes del multiplier), y el multiplier aplique después
- **Detalle:** Ver `mod-etapa-7-pulido.md` sección "Bug fix: Cap anti-power-leveling"

### 2025-05-24: Object graphics crash (nivel 3+)

- **Bug:** Crash al entrar a nivel 3 (Catacombs) — `Unable to find object_graphic_id 25, 26, 32, 33` → Segmentation fault
- **Causa:** `InitObjectGFX()` usa `currlevel` para comparar con `minlvl`/`maxlvl` de objetos (basados en niveles originales 1-16). Con niveles 1-7 del mod, los rangos nunca coinciden para Catacombs (minlvl=5), Caves (minlvl=9) o Hell (minlvl=13).
- **Fix:** `currlevel` → `GetVirtualLevel()` en la comparación de rangos del loop de `InitObjectGFX()`. Mismo patrón que el fix de storybooks.
- **Detalle:** Ver `mod-etapa-5-items.md` sección "Bug fix: InitObjectGFX() level-range loading"

### 2025-05-24: Quest reorganization

- **Cambio:** PWATER desactivada (no funcionaba en Catacombs), Q_BLOOD reactivada en nivel 4, Q_BLIND movida a nivel 3
- **Cambio:** Leoric (Q_SKELKING) ahora siempre usa set level, incluyendo multiplayer
- **Archivos:** `quests.cpp` (QuestsData[], InitQuests, CheckQuests), `drlg_l1.cpp` (LoadQuestSetPieces)
- **Detalle:** Ver `mod-etapa-4-quests.md` sección "Quest reorganization"

### 2025-05-24: Catacombs quest room crash (nivel 4)

- **Bug:** Crash al entrar al nivel 4 — `bitset::set: __position (which is 1610) >= _Nb (which is 1600)`
- **Causa:** `CreateDungeon()` en `drlg_l2.cpp` usaba `currlevel` directamente en un switch para asignar tamaños de sala a quests. Después del swap Q_BLIND→nivel 3 / Q_BLOOD→nivel 4, el switch no se actualizó — nivel 4 no creaba sala para Q_BLOOD, causando que el set piece se colocara en coordenadas inválidas
- **Fix:** Actualizado el switch: case 3 → Q_BLIND (15x15), case 4 → Q_BLOOD (14x20)
- **Detalle:** Ver `mod-etapa-6-dungeon.md` sección "Bug fix: Catacombs quest room sizing"

### 2025-05-24: Set levels en multiplayer — monstruos duplicados y re-entrada

- **Bug 1:** Leoric aparecía directamente en nivel 2 y Lazarus aparecía directamente en nivel 6, en vez de dentro de sus set levels
- **Bug 2:** Después de matar a Lazarus, la pentagrama que debería llevar a nivel 7 (Diablo) enviaba de vuelta a Lazarus Lair
- **Causa 1:** `PlaceQuestMonsters()` en `monster.cpp` colocaba a Leoric y Lazarus directamente en el nivel principal en MP, además de dentro de sus set levels
- **Causa 2:** `CheckQuests()` no excluía quests con `_qactive == QUEST_DONE`, así que al pisar la pentagrama (posición de Q_BETRAYER) te enviaba al set level aunque ya estaba completado
- **Fix:**
  - `monster.cpp`: Eliminada colocación directa de Leoric en nivel 2 (MP) y Lazarus+esbirros en nivel 6 (MP). Solo se colocan dentro de sus respectivos set levels (`SL_SKELKING`, `SL_VILEBETRAYER`)
  - `quests.cpp`: Agregado `quest._qactive != QUEST_DONE` en ambas ramas (SP y MP) de `CheckQuests()` — evita re-entrar a set levels completados
- **Detalle:** Ver `mod-etapa-4-quests.md` sección "Set levels fix en multiplayer"

### 2025-05-25: Lazarus quest flow — Staff of Lazarus + portal rojo en MP

- **Bug:** En MP, Lazarus no aparecía en ningún lado (se eliminó su colocación directa en nivel 6) y no había forma de entrar a Lazarus Lair
- **Causa:** El juego original MP no usa set level para Lazarus — lo coloca directamente en el nivel. Al eliminar eso para usar set level (como SP), se rompió el flujo completo de activación de la quest
- **Fix completo (5 archivos):**
  - `objects.cpp`: Pedestal del Staff (`AddLazStand()`) ahora se genera en MP también (antes solo SP)
  - `quests.cpp` InitQuests(): Eliminado `_qvar1 = 2` en MP — la quest empieza desactivada (necesita Staff + Cain)
  - `quests.cpp` ResyncMPQuests(): Eliminado auto-activate de Q_BETRAYER al pasar por nivel 5 + eliminado altar decorativo (`OBJ_ALTBOY`)
  - `quests.cpp` CheckQuests(): Eliminado altar (`OBJ_ALTBOY`) de nivel 6 + portal rojo se genera cuando `_qvar2 == 0` y `_qactive == QUEST_ACTIVE`
  - `towners.cpp` TalkToStoryteller(): Cain ahora acepta el Staff of Lazarus en MP y activa la quest (igual que SP)
- **Flujo resultante:** Staff en nivel 6 → hablar con Cain → portal rojo en pentagrama → Lazarus Lair → matar Lazarus → volver → pentagrama lleva a nivel 7
- **Sincronización MP:** Un jugador habla con Cain → `NetSendCmdQuest()` sincroniza para todos. Mismos mecanismos que SP.
- **Detalle:** Ver `mod-etapa-4-quests.md` sección "Lazarus quest flow completo (Staff + Cain + portal rojo)"

### 2025-05-25: Lazarus Lair MP hotfixes (5 fixes)

- **Bug:** Múltiples problemas en el flujo de Lazarus Lair en multiplayer
- **Fixes:**
  1. Portal rojo: `_qvar1 = 3` al spawn del portal → permite entrada al set level (`quests.cpp:293`)
  2. Lazarus no ataca: `_qvar1 <= 3` → `_qvar1 == 4` en `LazarusAi()` → Lazarus inicia diálogo solo en el estado correcto (`monster.cpp:2801`)
  3. Sala trasera no revelada: `ObjChangeMap(1,18,20,24)` + `RedoPlayerVision()` en `MonsterTalk()` Lazarus MP (`monster.cpp:1427`)
  4. Salida Lazarus Lair: `dPiece==369` (no existe en Cathedral) reemplazado por `InitVPTriggers()` + RedPortal en (35,32) (`quests.cpp:419`)
  5. Entradas de set levels: eliminado early return en `ForceQuests()` MP → labels visibles al cursor (`quests.cpp:370`)
- **Detalle:** Ver `mod-etapa-4-quests.md` sección "Lazarus Lair MP hotfixes"

### 2025-05-25: Pentagrama blanca/roja prematura en nivel 6 MP

- **Bug:** La pentagrama de Diablo en nivel 6 aparecía con sprite abierto desde el inicio en MP, antes de completar Lazarus
- **Causa:** `isGateOpen = UseMultiplayerQuests() || Quests[Q_DIABLO]._qactive == QUEST_ACTIVE` — `UseMultiplayerQuests()` siempre true en MP
- **Fix:** Eliminado `UseMultiplayerQuests()` de la condición — solo depende de `Quests[Q_DIABLO]._qactive` (`drlg_l4.cpp:1183`)
- **Detalle:** Ver `mod-etapa-6-mazmorras.md` sección "Bug fix: Pentagrama blanca/roja prematura"

### 2025-05-25: Flash spell — misil permanente

- **Bug:** Flash causaba sprite cortado + invulnerabilidad eterna al jugador
- **Causa:** `AddFlashBottom()` no inicializaba `_mirange` → misil nunca se eliminaba → colisión permanente
- **Fix:** `missile._mirange = 19` en `AddFlashBottom()` (`missiles.cpp:2057`)
- **Nota:** Bug de devilutionX original, no del mod
- **Detalle:** Ver `mod-fix-flash-spell.md`

### 2025-06-06: Skeleton slots en set levels

- **Bug:** En set levels (Leoric Chamber, Lazarus Lair), monstruos del `.dun` ocupaban slots 4-7 (reservados para esqueletos invocados). Causaba: (1) monstruos del .dun aparecían como "esqueletos" con HP base, (2) Raise Skeleton creaba minion con sprite incorrecto (arquero en vez de MT_WSKELAX)
- **Causa:** `SetMapMonsters()` colocaba monstruos del .dun en slots 4+ antes de que `InitSkeletons()` reservara los slots. No existe mecanismo de reserva — el código dependía del orden de ejecución, que cambia en set levels
- **Fix (4 archivos):**
  - `SetMapMonsters()` (`monster.cpp`): reserva slots 4-7 para esqueletos ANTES de monstruos del .dun
  - `InitSkeletons()` (`monster.cpp`): skip en set levels (`if (setlevel) return`)
  - `SpawnSkeleton()` (`monster.cpp`): fuerza tipo `MT_WSKELAX` con `InitMonster()` si el slot tiene tipo incorrecto
  - `DeltaLoadLevel()` (`msg.cpp`): check `monster.type().type == MT_WSKELAX` antes de aplicar `GolumAi` a slots 4-7
