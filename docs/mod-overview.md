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

## Decisiones de diseño

- `NUMLEVELS` se mantiene en 25 para evitar buffer overflows en código de Hellfire
- Código de Hellfire no se elimina, solo no se activa (`gbIsHellfire == false`)
- Lazarus es un **set level** (`SL_VILEBETRAYER`) — portal desde nivel 6, no ocupa un `currlevel`
- Diablo boss se coloca via `GetLevelMTypes()` hardcoded, no via `minDunLvl`/`maxDunLvl`
- Item generation usa mapeo virtual en vez de modificar `iMinMLvl` en `itemdat.cpp`
- El nivel 6 (Hell) hace doble función: es el primer nivel de Hell (town warp) Y el nivel de Lazarus (pentagrama)

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
