# Mod Diablo 7 Niveles — Referencia para Sesiones Nuevas

## Resumen del proyecto

Mod de Diablo 1 usando devilutionX. Reduce la mazmorra de 16 a 7 niveles, XP x10, XP compartida. Distribución 2+2+1+2 (Cathedral+Catacombs+Caves+Hell).

## Distribución de niveles

| Nivel mod | Tema | Nivel original | Quest |
|---|---|---|---|
| 1 | Cathedral | 1 | Butcher (Q_BUTCHER) |
| 2 | Cathedral | 3 | Skeleton King (Q_SKELKING) — set level SL_SKELKING |
| 3 | Catacombs | 5 | Halls of the Blind (Q_BLIND) |
| 4 | Catacombs | 7 | Valor (Q_BLOOD) |
| 5 | Caves | 9 | Anvil of Fury (Q_ANVIL) |
| 6 | Hell | 13 | Lazarus (Q_BETRAYER) — set level SL_VILEBETRAYER via portal rojo |
| 7 | Hell | 16 | Diablo (Q_DIABLO) |

## Mapeo virtual

```
LevelMap = { 0, 1, 3, 5, 7, 9, 13, 16 }
GetVirtualLevel() en Source/levels/gendung.h
```

## Constantes clave

- `NUMLEVELS` = 25 (sin cambio, seguridad Hellfire)
- `giNumberOfLevels` = 8 (7 niveles + town)
- `ExperienceMultiplier` = 10 (en player.cpp)
- Diablo original only, no Hellfire

## Archivos principales modificados

| Archivo | Qué hace |
|---|---|
| `Source/levels/gendung.h` | `GetVirtualLevel()`, `GetVirtualLevelForShop()` |
| `Source/levels/gendung.cpp` | `GetLevelType()` remapeado |
| `Source/diablo.cpp` | `giNumberOfLevels = 8`, cutscenes |
| `Source/levels/trigs.cpp` | Warps, escaleras, triggers (currlevel == 3/5/6) |
| `Source/levels/town.cpp` | Spawn positions para warps |
| `Source/interfac.cpp` | Cutscenes (nivel 7 = Diablo) |
| `Source/monstdat.cpp` | 93 monstruos remapeados (minDunLvl/maxDunLvl) |
| `Source/monster.cpp` | `GetLevelMTypes()`, `InitMonsters()`, Lazarus/Leoric removidos de nivel principal en MP |
| `Source/quests.cpp` | `QuestsData[]` (Q_BLIND→3, Q_BLOOD→4), `InitQuests()` (9 quests desactivadas), `CheckQuests()` (portal rojo MP, anti-re-entrada), `ResyncMPQuests()` (sin auto-activate Lazarus) |
| `Source/towners.cpp` | `_pLvlVisited` dinámico, Cain acepta Staff en MP |
| `Source/objects.cpp` | Story books (2,4,6), `InitObjectGFX()` con GetVirtualLevel(), Staff pedestal en MP, `AddLazStand()` sin guard MP |
| `Source/items.cpp` | `ItemsGetCurrlevel()` con mapeo virtual |
| `Source/levels/drlg_l1.cpp` | Leoric set piece habilitado en MP (sin `!UseMultiplayerQuests()`) |
| `Source/levels/drlg_l2.cpp` | Quest room sizing: case 3=Q_BLIND(15x15), case 4=Q_BLOOD(14x20), town warp nivel 3 |
| `Source/levels/drlg_l3.cpp` | Town warp nivel 5 |
| `Source/levels/drlg_l4.cpp` | 11 checks: Lazarus lvl 6, Diablo lvl 7, PlaceStairs reestructurado, pentagrama+town warp en lvl 6 |
| `Source/levels/themes.cpp` | 3 checks currlevel==16 → 7 |
| `Source/missiles.cpp` | 15 damage formulas + minhit con GetVirtualLevel() |
| `Source/player.cpp` | XP x10, shared XP (gbActivePlayers), voice dialogs (1/3/5/6/7), trap disarm |
| `Source/stores.cpp` | Shop level mapping con GetVirtualLevelForShop() |
| `Source/control.cpp` | `glSeedTbl[7]` |

## Flujo de quests con set level

### Leoric (nivel 2, SL_SKELKING) — SP y MP

1. Nivel 2 genera tumba (`skngdo.dun`) como set piece (habilitado en MP)
2. `CheckQuests()` detecta jugador en `quest.position` → entra a SL_SKELKING
3. Dentro: Leoric se coloca via `PlaceUniqueMonst(SkeletonKing)` (monster.cpp:542)
4. Mata a Leoric → vuelve a nivel 2

### Lazarus (nivel 6, SL_VILEBETRAYER) — SP y MP

1. Nivel 6 genera pedestal del Staff (`OBJ_LAZSTAND`) con Staff of Lazarus
2. Jugador obtiene el Staff, va a pueblo, habla con Cain → quest activada (`_qvar1 = 2`)
3. Vuelve a nivel 6 → portal rojo aparece en la pentagrama
4. Pisa el portal → entra a SL_VILEBETRAYER
5. Dentro: Lazarus habla (LazarusAi, _qvar1<=3) → se vuelve hostil → combate → muerte
6. `_qactive = QUEST_DONE`, `_qvar1 = 7` → pentagramas se registran como WM_DIABNEXTLVL
7. Portal rojo dentro del set level → vuelve a nivel 6
8. Pentagrama lleva a nivel 7 (Diablo)

## Quests eliminadas (9)

Q_ROCK, Q_MUSHROOM, Q_GARBUD, Q_ZHAR, Q_VEIL, Q_LTBANNER, Q_PWATER, Q_WARLORD, Q_SCHAMB

Forzadas a QUEST_NOTAVAIL en InitQuests().

## _qvar1 de Q_BETRAYER — estados

| Valor | Significado |
|---|---|
| 0 | Inicial (InitQuests) — quest no activada |
| 2 | Quest activada (Cain tomó el Staff) |
| 3 | CheckQuests procesó nivel 6 (SP: portal rojo generado) |
| 4 | Lazarus inicia diálogo (dentro de SL_VILEBETRAYER, SP: ObjChangeMap) |
| 5 | Lazarus hablando (SP: movie fprst3.smk) |
| 6 | Lazarus terminó diálogo → hostil (MonsterTalk en MP, ObjChangeMap+RedoPlayerVision en SP) |
| 7 | Lazarus muerto (OnMonsterDeath) |
| 8 | Cain post-quest dialog (TalkToStoryteller) |

## _qvar2 de Q_BETRAYER — portal rojo

| Valor | Significado |
|---|---|
| 0 | Inicial — portal rojo aún no generado |
| 1 | Portal rojo generado en pentagrama (nivel 6) |
| 2 | ResyncQuests detectó _qvar2==1 o >=3 → proceso de limpieza |
| 3 | Dentro de SL_VILEBETRAYER, Lazarus activo |
| 4 | Lazarus muerto → portal rojo dentro del set level |

## Hotfixes aplicados (orden cronológico)

1. **Storybook crash (nivel 2)**: InitObjectGFX storybook check 4,8,12 → 2,4,6
2. **XP multiplier MP**: Cap anti-power-leveling aplica antes del multiplier
3. **Object graphics crash (nivel 3+)**: InitObjectGFX level-range loop usa GetVirtualLevel()
4. **Quest reorganization**: Q_PWATER desactivada, Q_BLOOD→nivel 4, Q_BLIND→nivel 3, Leoric set level en MP
5. **Catacombs quest room crash (nivel 4)**: drlg_l2.cpp CreateDungeon() switch actualizado
6. **Set levels MP — monstruos duplicados + re-entrada**: monster.cpp Leoric/Lazarus removidos de nivel principal, CheckQuests _qactive != QUEST_DONE
7. **Lazarus quest flow completo**: Staff+Cain en MP, portal rojo, altar eliminado de nivel 6

## Decisiones de diseño

- `NUMLEVELS` = 25 (seguridad buffers Hellfire)
- Hellfire code intacto, solo no se activa
- Item generation usa mapeo virtual (no modifica itemdat.cpp)
- Nivel 6 hace doble función: primer Hell (town warp) + Lazarus (pentagrama)
- Lazarus quest flow idéntico a SP original: Staff → Cain → portal rojo → set level → muerte → Diablo
- Un jugador activa quest con Cain → `NetSendCmdQuest()` sincroniza para todos en MP
- `CheckQuests()` no re-envía a set levels completados (`_qactive != QUEST_DONE`)

## Qué falta / pendientes de testing

- Verificar que Lazarus quest funcione end-to-end en MP (Staff → Cain → portal → set level → muerte → Diablo)
- Verificar que Lazarus funcione correctamente en SP
- Verificar Leoric set level en MP
- Verificar que items generados en nivel 6-7 tengan poder adecuado
- Verificar estabilidad de todos los niveles en MP cooperativo (2+ jugadores)
- Balance general: ¿7 niveles con x10 XP es demasiado rápido?

## Cómo compilar

```bash
cmake --build build -j$(nproc)
```

El binario se genera en `build/devilutionx`.
