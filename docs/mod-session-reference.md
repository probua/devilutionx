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
| `Source/minion_ai.cpp` / `.h` | `GolumAi`, `ScanForEnemy()`, `MoveToward()`, `ActivateNearbyMonsters()`, `ClearMinionTarget()`, `InitGolems`, `InitSkeletons`, `PreSpawnSkeleton`, `SpawnGolem`, `SpawnSkeleton` (movidos desde `monster.cpp`) |
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

## Cambios de hechizos (sesión 2025-06-06)

### Healing + HealOther — fusión

- HealOther reemplazado por `Null` en spellbook (página 0, slot 5)
- `GetBookSpell()` y `GetStaffSpell()` siempre saltan HealOther
- Al leer libro de Healing, también se aprende HealOther al mismo nivel (vía `CMD_CHANGE_SPELL_LEVEL`)
- Scrolls de Heal Other: sin cambios, siguen siendo funcionales
- Docs: `docs/spells/healing-healother-merge.md`

### Telekinesis — rework

- Agregado al spellbook (página 0, slot 5, donde antes estaba HealOther)
- Knockback multi-tile (loop de 2 `M_GetKnockback`)
- Stun ~1 segundo vía `MonsterMode::Delay` con var2=30
- No causa daño, no escala por nivel de hechizo
- Docs: `docs/spells/telekinesis-rework.md`

### Raise Skeleton — nuevo hechizo

- `SpellID::Skeleton` en posición 37 (entre `BoneSpirit=36` y `Mana=38`)
- `LastDiablo = Skeleton` (37), `MAX_SPELLS = 53`, `MissileID::Skeleton = 108`
- Mana cost 50, tipo `Magic | Targeted`, `sBookLvl=1`, `sStaffLvl=9`, `minInt=40`
- Invoca esqueleto en `Monsters[MAX_PLRS + playerId]` (slots 4–7), misma IA que Golem (`GolumAi`)
- IA corregida: `ownerId = getId() - MAX_PLRS` para esqueletos
- Multiplayer completo: `CMD_AWAKESKELETON`, `NetSendCmdSkeleton`, `OnAwakeSkeleton`, `DeltaSyncSkeleton`
- Cleanup en `RemovePlrMissiles()` al cambiar de nivel
- Docs: `docs/spells/raise-skeleton.md`

### Fix maxSpells (GetBookSpell / GetStaffSpell)

- `items.cpp:641,1284`: `maxSpells` cambiado de `37` a `static_cast<int>(SpellID::LastDiablo) + 1` (=38)
- El wrap `if (s == maxSpells) s = 1` ahora ocurre en `Mana` (pos 38, `sBookLvl=-1`) en vez de en `Skeleton` (pos 37)
- Raise Skeleton ahora es elegible como libro y como enchantment de staff

### Spell tiers — sistema de niveles de libros

- 4 tiers alineados con las 4 páginas del spellbook
- Página 0 (Tier 1, sBookLvl=1): Firebolt, ChargedBolt, HolyBolt, Healing, Telekinesis, Skeleton
- Página 1 (Tier 2, sBookLvl=3): FireWall, Inferno, Lightning, TownPortal, Flash, StoneCurse
- Página 2 (Tier 3, sBookLvl=5): Phasing, ManaShield, Elemental, Fireball, FlameWave, ChainLightning, Guardian
- Página 3 (Tier 4, sBookLvl=6): Nova, Golem, Teleport, Apocalypse, BoneSpirit, BloodStar
- `iMinMLvl` de libros base: BOOK2 8→6, BOOK3 14→10, BOOK4 20→12
- Color de libro según tipo de magia: `Fire`→rojo, `Lightning`→azul, `Magic`→gris
- Golem y Raise Skeleton cambiados a `Magic | Targeted` (libros grises)
- Sistema acumulativo: tiers inferiores siempre disponibles, superiores se agregan al avanzar
- Docs: `docs/spells/spell-tiers.md`

### MaxSpellLevel: 15 → 4

- `player.h:37`: `MaxSpellLevel` cambiado de 15 a 4
- Afecta todos los hechizos: libros se leen hasta nivel 4, shrines capped, red/save clamped
- `ScaleSpellEffect(base, sl)`: max sl=4 → ×1.60 (vs ×5.56 antes)
- Rebalanceo pendiente — las fórmulas de daño/duración/healing están calibradas para sl 1–15

## Datos de hechizos página 0

| SpellID | Mana base | sBookLvl | sStaffLvl | minInt | Stat requisito | sManaAdj | sMinMana |
|---|---|---|---|---|---|---|---|
| Firebolt | 6 | 1 | 1 | 15 | Magia | 1 | 3 |
| ChargedBolt | 6 | 1 | 1 | 25 | Magia | 1 | 6 |
| HolyBolt | 7 | 1 | 1 | 20 | Magia | 1 | 3 |
| Healing | 5 | 1 | 1 | 25 | **Vitality** | 3 | 1 |
| Telekinesis | 15 | 1 | 2 | 30 | **Dexterity** | 2 | 8 |
| Raise Skeleton | 50 | 1 | 9 | 40 | Magia | 6 | 60 |
| Golem | 100 | 6 | 9 | 81 | Magia | 6 | 60 |

## Requisitos de hechizo por stat alternativo (Vitality / Dexterity)

Algunos hechizos exigen un stat distinto a Magia para aprender/usar (libros y báculos). El **valor** del requisito sigue en `SpellData::minInt` (y por tanto en `Item::_iMinMag`, que se persiste y escala +20%/nivel sin cambios); lo que cambia es **contra qué stat se compara** ese valor.

- **`SpellRequirementStat GetSpellRequirementStat(SpellID)`** (`Source/spelldat.h` / `spelldat.cpp`): enum `{ Magic, Vitality, Dexterity }`. Default `Magic`; `Healing`/`HealOther → Vitality`, `Telekinesis → Dexterity`.
- **`Player::CanUseItem`** (`Source/player.cpp`): para `IMISC_BOOK` y `ItemType::Staff`, compara `_iMinMag` contra el stat indicado por el helper.
- **`CalcSelfItems`** (`Source/items.cpp`): añade tracking de vitalidad (`_iPLVit` + `_pBaseVit`) y check stat-aware (relevante para báculos equipados).
- **`SpawnOnePremium`** (`Source/items.cpp`): añade vitalidad y check stat-aware en el filtro Hellfire.
- **Display** (`Source/items.cpp:PrintItemInfo`, `Source/stores.cpp`): reetiqueta "Mag" → "Vit"/"Dex" en libros/báculos.
- **Sin cambios:** generación de items, escalado +20%/nivel, save/load (no se añade campo al formato de guardado).
- **Scrolls** siguen sin requisito de stat. El escalado +20%/nivel se mantiene (p.ej. Healing lvl2 → 30 Vit).

Para añadir más hechizos con requisito alterno: agregar el `case` en `GetSpellRequirementStat()` y ajustar el `minInt` en `spelldat.cpp`.

## Libros de hechizo en nivel máximo (mastered)

Cuando un libro pertenece a un hechizo ya en `MaxSpellLevel` (4), el tooltip **no** muestra el requisito del "siguiente nivel" (que sería inalcanzable). En su lugar muestra el mismo mensaje que al click derecho: *"You have already mastered this spell"* (`EMSG_SPELL_MAXED`). Además el nombre del libro no se muestra en rojo.

- **`IsSpellBookMaxed(const Item&, const Player&)`** (declarado en `Source/items.h`, definido en `Source/items.cpp`): espejo del guard de click derecho (`inv.cpp`/`stash.cpp`). Verifica `_pSplLvl[spell] >= MaxSpellLevel`; para Healing también requiere HealOther al máximo (fusión).
- **`PrintItemMisc`** (`Source/items.cpp`): si el libro está maximizado, muestra el mensaje "mastered" en vez de "Right-click to read".
- **`PrintItemInfo`** (`Source/items.cpp`): si está maximizado, omite la línea "Required: …".
- **`updateRequiredStatsCacheForPlayer`** (`Source/items.cpp`): si está maximizado, setea `_iStatFlag = true` (color normal) y no escala `_iMinMag`.
- **Tienda de Adria** (`Source/stores.cpp`): `PrintStoreItem` muestra el mensaje "mastered" para libros maximizados (compra y venta), y `StartWitchBuy` fuerza `_iStatFlag = true` (color normal).
- Cubre inventario, stash y tienda de Adria.

## Velocidad de casteo por clase

La velocidad de casteo = `castingFrames` en `PlayersAnimData[]` (`Source/playerdat.cpp`), con `ticksPerFrame=1`. Menos frames = más rápido.

| Clase | castingFrames | castingActionFrame | ~Tiempo (20 fps) |
|---|---|---|---|
| Warrior | 16 | 12 | 0.8s (antes 20/1.0s) |
| Rogue | 16 | 12 | 0.8s |
| Sorcerer | 12 | 8 | 0.6s |

El Guerrero se igualó a la Rogue (16 frames). Bard/Barbarian sin cambios (inactivos, `gbIsHellfire=false`).

## Hechizos iniciales por clase

En `CreatePlayer()` (`Source/player.cpp`), cada clase empieza con hechizos conocidos (`_pMemSpells`) y nivel de hechizo (`_pSplLvl`). El hechizo seleccionado por defecto (`_pRSpell`/`_pRSplType`) sigue siendo el skill de clase (ItemRepair/TrapDisarm/StaffRecharge), excepto el Sorcerer que selecciona Firebolt.

| Clase | Skill | Hechizos iniciales (lvl 1) |
|---|---|---|
| Warrior | ItemRepair | **Healing** + HealOther (fusión) |
| Rogue | TrapDisarm | **Telekinesis** |
| Sorcerer | StaffRecharge | **Firebolt** (vanilla) |

- **Sólo personajes nuevos.** Saves existentes no reciben los hechizos retroactivamente.
- HealOther se incluye en `_pMemSpells` y `_pSplLvl` para ser consistente con la fusión (al leer un libro de Healing, ambos suben de nivel via `CMD_CHANGE_SPELL_LEVEL`). HealOther está oculto del spellbook layout.
- Stats verificadas: Warrior baseVit=25 (= req. Healing), Rogue baseDex=30 (= req. Telekinesis). Mana suficiente para castear a lvl 1.

## Archivos modificados (hechizos)

| Archivo | Cambio |
|---|---|
| `Source/player.h` | `MaxSpellLevel = 4` |
| `Source/spelldat.h` | `Skeleton=37`, `LastDiablo=Skeleton`, `MAX_SPELLS=53`, `MissileID::Skeleton=108` |
| `Source/spelldat.cpp` | Datos de Raise Skeleton, Golem/Skeleton a Magic, sBookLvl de todos ajustados |
| `Source/items.cpp` | GetBookSpell/GetStaffSpell (maxSpells fix), GetBookSpell/GetStaffSpell siempre saltan HealOther, Healing book sube HealOther |
| `Source/itemdat.cpp` | iMinMLvl de BOOK2/BOOK3/BOOK4 reducidos |
| `Source/panels/spell_book.cpp` | Layout página 0 y 1 (Skeleton, Telekinesis, Null, Inferno) |
| `Source/panels/spell_icons.cpp` | `SpellITbl[37] = 24`, array 53 entradas |
| `Source/misdat.cpp` | `MissilesData[Skeleton]` con AddSkeleton |
| `Source/missiles.cpp` / `.h` | `AddSkeleton()` |
| `Source/monster.cpp` / `.h` | skeletonTypeIndex, KillMySkeleton, DeleteMonsterList skeleton loop, SetMapMonsters slot reservation, golem idle freeze |
| `Source/minion_ai.cpp` / `.h` | `GolumAi` (3 estados via `var1`), `PickMinionTarget`, `MoveToward`, `ActivateNearbyMonsters`, `InitGolems`, `InitSkeletons`, `PreSpawnSkeleton`, `SpawnGolem`, `SpawnSkeleton` (movidos desde `monster.cpp`) |
| `Source/automap.cpp` | DrawAutomapMinion — golem y esqueleto visibles en automapa (flecha verde) |
| `Source/qol/minionstatus.cpp` / `.h` | DrawMinionStatus — HUD centrado con icono + barra HP + debug state para golem/esqueleto |
| `Source/engine/render/scrollrt.cpp` | Llamada a DrawMinionStatus después de DrawXPBar |
| `Source/msg.cpp` / `.h` | CMD_AWAKESKELETON, NetSendCmdSkeleton, OnAwakeSkeleton, DeltaSyncSkeleton, OnKillGolem modified, DeltaLoadLevel type check en slots 4-7 |
| `Source/player.cpp` | RemovePlrMissiles esqueleto cleanup; `CanUseItem` stat-aware (Vit/Dex); `CreatePlayer` hechizos iniciales Warrior=Healing+HealOther, Rogue=Telekinesis |
| `Source/diablo.cpp` | InitSkeletons() en load de nivel |

## Qué falta / pendientes de testing

- Verificar que Lazarus quest funcione end-to-end en MP (Staff → Cain → portal → set level → muerte → Diablo)
- Verificar que Lazarus funcione correctamente en SP
- Verificar Leoric set level en MP
- Verificar que items generados en nivel 6-7 tengan poder adecuado
- Verificar estabilidad de todos los niveles en MP cooperativo (2+ jugadores)
- Balance general: ¿7 niveles con x10 XP es demasiado rápido?
- Rebalancear fórmulas para `MaxSpellLevel=4` (ScaleSpellEffect, daños, summons, mana cost, Mana Shield, duraciones)
- Probar Raise Skeleton end-to-end (spawn, IA, cleanup, MP sync)
- Probar Raise Skeleton en set levels (Leoric Chamber, Lazarus Lair)

## Fix: Skeleton slots en set levels

**Bug**: En set levels (Leoric Chamber, Lazarus Lair), `SetMapMonsters()` colocaba monstruos del `.dun` en slots 4-7 (reservados para esqueletos invocados). Esto causaba:
1. Monstruos del .dun aparecían como "esqueletos" con HP base (5 HP)
2. Al invocar Raise Skeleton, el spell operaba sobre el slot equivocado → aparecía con sprite de arquero en vez de MT_WSKELAX

**Root cause**: No existe mecanismo de reserva de slots. En niveles normales, `InitGolems()` + `InitSkeletons()` corren antes que `InitMonsters()` (slots 0-7 reservados). En set levels, `SetMapMonsters()` corre primero y llena slots 4+ con monstruos del .dun.

**Fix** (4 cambios):

1. **`SetMapMonsters()`** (`monster.cpp`): Reservar slots 4-7 para esqueletos ANTES de procesar la monster layer del .dun. Los monstruos del .dun ahora empiezan en slot 8+.
2. **`InitSkeletons()`** (`monster.cpp`): Skip en set levels (`if (setlevel) return`) — ya creados por `SetMapMonsters`.
3. **`SpawnSkeleton(Player&, Monster&, ...)`** (`monster.cpp`): Forzar tipo `MT_WSKELAX` con `InitMonster()` si el slot tiene tipo incorrecto (defensa contra deltas antiguos).
4. **`DeltaLoadLevel()`** (`msg.cpp`): Agregar check `monster.type().type == MT_WSKELAX` antes de aplicar `GolumAi` a slots 4-7 (defensa contra deltas de saves antiguos).
- Probar Healing+HealOther fusion (libro de Healing sube ambos)
- Probar Telekinesis rework (knockback 2 tiles + stun)
- Verificar dropeo de libros por tier en cada nivel de mazmorra
- Agregar stubs para hechizos no implementados (DoomSerpents, BloodRitual, Invisibility) si crashean
- Probar IA de leash de golem/esqueleto (ScanForEnemy, MoveToward, estados via goal)

## Minion AI (golem/esqueleto)

La IA `GolumAi` vive en `Source/minion_ai.cpp` / `Source/minion_ai.h` (separado de `monster.cpp`). Usa `Monster::goal` para el estado principal y `var1` para sub-estado.

### States (using `Monster::goal`)

| `goal` | var1 | Name | When | Behavior |
|---|---|---|---|---|
| `MonsterGoal::Normal` | 2 (Idle) | IDLE | Default state | Scans for enemies via `ScanForEnemy()`, faces owner. Transitions to FOLLOW if owner >8 tiles, or CHASE if enemy found |
| `MonsterGoal::Move` | 0 (Follow) | FOLLOW | Owner >8 tiles away | Walks toward owner with 4-tick delay. Transitions to IDLE when owner ≤6 tiles (hysteresis) |
| `MonsterGoal::Attack` | 1 (Chase) | CHASE | Enemy visible in same room | Pursues fixed target. Transitions to FOLLOW if owner >8 tiles, IDLE if target dies or >10 tiles away |

### Transitions
```
IDLE ──(owner >8)──→ FOLLOW
IDLE ──(enemy visible)──→ CHASE
FOLLOW ──(owner ≤6, no enemy)──→ IDLE
FOLLOW ──(owner ≤6, enemy visible)──→ CHASE
CHASE ──(owner >8)──→ FOLLOW
CHASE ──(target dead or >10 tiles)──→ IDLE
```

### Vision (ScanForEnemy)
- Only called from IDLE state (not every tick)
- Filters: `dTransVal` (same room) + `LineClearMissile` (line of sight)
- Minion cannot detect enemies through walls or doors
- Prioritizes monsters attacking the owner (threats) within 5 tiles of owner
- Then nearest enemy within 8 tiles of minion
- Sets `MFLAG_TARGETS_MONSTER`, `enemy`, `enemyPosition` when found

### ClearMinionTarget

Cuando el minion pierde su target (target muere, se aleja >10 tiles, o dueño >8 tiles), `ClearMinionTarget()` resetea `enemy` al `ownerId` y `enemyPosition` a la posición del dueño. Esto mantiene consistencia con el assert de `ProcessMonsters()` que espera `monster.enemy < MAX_PLRS` cuando `MFLAG_TARGETS_MONSTER` está limpio.

### Sticky target
- Target is stored in `monster.enemy` and persists across ticks
- Only recalculated when entering CHASE from IDLE
- Lost when target dies, target >10 tiles, or owner >8 tiles

### Constants

| Constant | Value | Purpose |
|---|---|---|
| `MaxMinionReturnDistance` | 8 | Owner distance to trigger FOLLOW |
| `MinionFollowHysteresis` | 6 | Owner distance to stop FOLLOW (hysteresis) |
| `MinionChaseMaxRange` | 10 | Max distance before losing target |
| `MinionIdleDelay` | 4 | Tick delay between steps in FOLLOW |
| `MinionEngageRange` | 5 | Range for threat detection near owner |

### Debug HUD estados

En builds debug (`_DEBUG`), el HUD lee `goal` y `var1` directamente:
- `MonsterGoal::Normal` + var1=2 → `IDLE`
- `MonsterGoal::Move` + var1=0 → `FOLLOW`
- `MonsterGoal::Attack` + var1=1 → `CHASE`
- Además: `ATTACK` / `WALK` / `DEAD` según `MonsterMode`

### Fix: IA consistente entre minions (AiPlanPath + PickMinionTarget)

**Bug**: El esqueleto nunca perseguía enemigos lejanos, y ambos minions se quedaban quietos cuando el path estaba despejado. El golem "arrancaba" después de matar pero el esqueleto no.

**Root causes**:
1. `AiPlanPath()` discriminaba por tipo (`MT_GOLEM`) en vez de por rol (minion del jugador). El esqueleto (`MT_WSKELAX`) abortaba por `activeForTicks == 0`.
2. El CHASE block de `GolumAi` no tenía fallback walk — solo `AiPlanPath`, que falla cuando el path está clear.
3. `UpdateEnemy()` nunca limpiaba `MFLAG_TARGETS_MONSTER` para minions → `GolumAi` nunca re-buscaba enemigos.

**Fix** (ahora en `Source/minion_ai.cpp`):
1. `AiPlanPath()`: `monster.type().type != MT_GOLEM` → `(monster.flags & MFLAG_GOLEM) == 0` (2 lugares). Ahora todo minion del jugador usa la misma ruta de código.
2. `SpawnSkeleton()` + `SpawnGolem()`: agregar `activeForTicks = UINT8_MAX`.
3. `GolumAi()` CHASE: `MoveToward()` encapsula `AiPlanPath` + `RandomWalk` fallback hacia enemigo.
4. `ScanForEnemy()` reemplaza `UpdateEnemy()` — selección centralizada desde IDLE, con filtros de sala (`dTransVal`) + línea de visión (`LineClearMissile`), sin flags `MFLAG_TARGETS_MONSTER` / `MFLAG_NO_ENEMY`.

### Idle freeze (golem)

El golem no tiene sprite de Stand en los assets originales (0 frames). Para que no se vea "corriendo" mientras está quieto, se usa el primer frame de la animación Attack congelado con `ticksPerFrame = 127` (máximo int8_t):

- **`MonsterIdle()`** (monster.cpp ~1018): cuando el golem está en Stand, `MonsterGraphic::Attack` frame 0 freeze
- **`M_StartStand()`** (monster.cpp ~3631): cuando el golem entra a Stand, `MonsterGraphic::Attack` frame 0 freeze
- Al caminar/atacar, `NewMonsterAnim` resetea `ticksPerFrame` al valor normal → animación se reanuda
- **Solo aplica a `MT_GOLEM`** — el esqueleto (`MT_WSKELAX`) tiene sprites Stand propios

Constantes definidas en `Source/minion_ai.cpp` (namespace anónimo).

## Minion en automapa (Tab)

El golem y el esqueleto del jugador local ahora se muestran en el automapa como flechas verdes:

- **Forma**: flecha (igual que los jugadores) que indica la dirección del minion
- **Color**: verde (`MapColorsMinion = PAL16_BEIGE + 8`)
- **Solo locales**: solo se muestran los minions del jugador local
- **Detección**: se dibujan si `position.tile != GolemHoldingCell`

Función `DrawAutomapMinion()` en `Source/automap.cpp`, llamada desde `DrawAutomap()` después de dibujar jugadores.

## Minion status HUD

HUD que muestra el estado del golem y esqueleto del jugador local, centrado horizontalmente justo arriba del panel principal:

- **Solo en dungeon**: no se dibuja en town (`leveltype == DTYPE_TOWN`)
- **Solo si vivo**: se dibuja solo si `position.tile != GolemHoldingCell`
- **Centrado**: 1 minion → centrado sobre el panel; 2 minions → lado a lado centrados como grupo
- **Layout horizontal**: Skeleton izquierda, Golem derecha, con 4px de separación
- **Sin minions**: no se dibuja nada

Cada cuadro (220x36px) contiene:
- **Icono del hechizo** (37x38px) — `DrawSmallSpellIcon` con `SpellID::Skeleton` o `SpellID::Golem`
- **Nombre** — "Skeleton" o "Golem" en `UiFlags::ColorWhite | FontSize12`
- **Barra de vida** (120x3px) — gradiente de 3 líneas, color según HP% (verde >60%, amarillo 30-60%, rojo <30%)
- **Texto HP** — `currHP/maxHP` centrado debajo de la barra

En builds debug (`_DEBUG`):
- **Estado AI** — tag `[STATE]` a la derecha del nombre en color whitegold (`ColorWhitegold`)
- Estados posibles: `IDLE`, `FOLLOW`, `CHASE`, `ATTACK`, `WALK`, `DEAD`
- Leído directamente de `var1` vía `MinionState` enum (sin replicar condiciones de `GolumAi`)

Función `DrawMinionStatus()` en `Source/qol/minionstatus.cpp`, llamada desde `DrawView` en `scrollrt.cpp` después de `DrawXPBar`.

## Debug commands (build con `-DCMAKE_BUILD_TYPE=Debug`)

Usar en chat del juego (solo en build debug):

| Comando | Descripción |
|---|---|
| `drop golem` | Busca ítem cuyo nombre contenga "golem" — mejor usar en dungeon, no en town |
| `drop raise skeleton` | Idem para Raise Skeleton (debe dropear como libro gris en nivel 1+) |
| `iteminfo` | Muestra info del ítem seleccionado (ID, seed, flags, validación) |

## Cómo compilar

```bash
cmake --build build-debug -j$(nproc)
```

Ejecutar con AddressSanitizer:

```bash
ASAN_OPTIONS=detect_leaks=0 ./build-debug/devilutionx
```

El binario se genera en `build-debug/devilutionx`.
