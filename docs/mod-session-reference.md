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

| SpellID | Mana base | sBookLvl | sStaffLvl | minInt | sManaAdj | sMinMana |
|---|---|---|---|---|---|---|
| Firebolt | 6 | 1 | 1 | 15 | 1 | 3 |
| ChargedBolt | 6 | 1 | 1 | 25 | 1 | 6 |
| HolyBolt | 7 | 1 | 1 | 20 | 1 | 3 |
| Healing | 5 | 1 | 1 | 17 | 3 | 1 |
| Telekinesis | 15 | 1 | 2 | 33 | 2 | 8 |
| Raise Skeleton | 50 | 1 | 9 | 40 | 6 | 60 |
| Golem | 100 | 6 | 9 | 81 | 6 | 60 |

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
| `Source/monster.cpp` / `.h` | skeletonTypeIndex, InitSkeletons, SpawnSkeleton, KillMySkeleton, GolumAi leash (MaxMinionChaseDistance/MaxMinionReturnDistance), DeleteMonsterList skeleton loop |
| `Source/automap.cpp` | DrawAutomapMinion — golem y esqueleto visibles en automapa (flecha verde) |
| `Source/qol/minionstatus.cpp` / `.h` | DrawMinionStatus — HUD con icono + barra HP para golem/esqueleto |
| `Source/engine/render/scrollrt.cpp` | Llamada a DrawMinionStatus después de DrawXPBar |
| `Source/msg.cpp` / `.h` | CMD_AWAKESKELETON, NetSendCmdSkeleton, OnAwakeSkeleton, DeltaSyncSkeleton, OnKillGolem modified, DeltaLoadLevel skeleton support |
| `Source/player.cpp` | RemovePlrMissiles esqueleto cleanup |
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
- Probar Healing+HealOther fusion (libro de Healing sube ambos)
- Probar Telekinesis rework (knockback 2 tiles + stun)
- Verificar dropeo de libros por tier en cada nivel de mazmorra
- Agregar stubs para hechizos no implementados (DoomSerpents, BloodRitual, Invisibility) si crashean
- Probar IA de leash de golem/esqueleto (dejar perseguir a 8, volver a 12)

## Minion leash (golem/esqueleto)

La IA `GolumAi` tiene tres estados de comportamiento:

### Constantes

| Constante | Valor | Descripción |
|---|---|---|
| `MaxMinionReturnDistance` | 8 | Distancia a la que el minion sigue al dueño |
| `MinionEngageRange` | 5 | Rango para detectar enemigos y entrar en combate |
| `MinionIdleDelay` | 4 | Ticks entre pasos en FOLLOW (~5 pasos/seg) |

### Estados

| Condición | Estado | Comportamiento |
|---|---|---|
| `distToOwner > 8` | **FOLLOW** | Pathfind hacia el dueño con delay de 4 ticks. Fallback a RandomWalk |
| `distToOwner ≤ 8` + enemigo a ≤5 tiles | **CHASE** | Pathfind hacia enemigo, atacar si adyacente |
| `distToOwner ≤ 8` + sin enemigo cercano | **IDLE** | Quieto, mirando al dueño |

### Transiciones

- `UpdateEnemy()` corre cada tick → detección de enemigos es instantánea (~50ms)
- `var2` se usa como contador de delay en FOLLOW
- Al detectar enemigo: `var2 = 0`, modo activo inmediato
- Al perder enemigo: transición a IDLE en el siguiente tick

### Pathfinding

`AiPlanPathTo(golem, ownerPosition)` usa BFS (`FindPath`) para navegar alrededor de paredes. Fallback a `RandomWalk` directo si no encuentra ruta. Aplica en FOLLOW.

### Debug HUD estados

En builds debug (`_DEBUG`), el tag `[STATE]` muestra:
- `FOLLOW` — distToOwner > 8
- `CHASE` — enemigo cercano a ≤5 tiles
- `IDLE` — sin enemigo, mirando al dueño
- `ATTACK` / `WALK` / `DEAD` — según MonsterMode

### Idle freeze (golem)

El golem no tiene sprite de Stand en los assets originales (0 frames). Para que no se vea "corriendo" mientras está quieto, se usa el primer frame de la animación Attack congelado con `ticksPerFrame = 127` (máximo int8_t):

- **`MonsterIdle()`** (monster.cpp ~1018): cuando el golem está en Stand, `MonsterGraphic::Attack` frame 0 freeze
- **`M_StartStand()`** (monster.cpp ~3631): cuando el golem entra a Stand, `MonsterGraphic::Attack` frame 0 freeze
- Al caminar/atacar, `NewMonsterAnim` resetea `ticksPerFrame` al valor normal → animación se reanuda
- **Solo aplica a `MT_GOLEM`** — el esqueleto (`MT_WSKELAX`) tiene sprites Stand propios

Constantes definidas en `Source/monster.cpp` (namespace anónimo).

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
- Estados posibles: `IDLE`, `FOLLOW`, `CHASE`, `RETREAT`, `URGENT`, `ATTACK`, `WALK`, `DEAD`
- Inferido por `GetMinionAiState()` replicando las mismas condiciones de `GolumAi`

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
