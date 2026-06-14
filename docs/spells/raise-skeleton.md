# Raise Skeleton — Nuevo hechizo de invocación

## Objetivo

Agregar un nuevo hechizo "Raise Skeleton" que invoca un esqueleto aliado controlado por IA. Es funcionalmente un segundo Golem con gráficos de esqueleto, usando su misma IA y su mismo slot de holding cell.

## Cambios

### 1. Enum y datos del hechizo (`Source/spelldat.h`, `Source/spelldat.cpp`)

- `SpellID::Skeleton` agregado en posición 37 (entre `BoneSpirit=36` y `Mana=38`)
- `LastDiablo = Skeleton` — válido en modo Diablo (pasa `IsValidSpell`)
- `MAX_SPELLS = 53`
- `MissileID::Skeleton` agregado al final del enum (valor 108)
- Datos del hechizo: mana cost 50, `Fire | Targeted`, bookLvl 11, staffLvl 9

### 2. Icono del hechizo (`Source/panels/spell_icons.cpp`)

- `SpellITbl[37] = 24` — frame del ícono de Apocalypse

### 3. Layout del libro de hechizos (`Source/panels/spell_book.cpp`)

- Página 0, slot 6: `SpellID::Skeleton`
- Inferno movido a página 1, slot 2 (antiguo slot de Telekinesis)

### 4. Datos del misil (`Source/misdat.cpp`)

- `MissilesData[Skeleton]`: `&AddSkeleton`, `nullptr`, `MissileGraphicID::None`, `Physical | Invisible`

### 5. Sistema de monstruos (`Source/monster.cpp`, `Source/monster.h`)

- **`skeletonTypeIndex`**: variable global que almacena el tipo de monstruo de los esqueletos invocados
- **`GetLevelMTypes()`**: registra `MT_WSKELAX` con `PLACE_SPECIAL` y guarda el índice en `skeletonTypeIndex`
- **`InitSkeletons()`**: pre-crea esqueletos en `GolemHoldingCell` para cada jugador (slots `MAX_PLRS + playerId`). **En set levels**, skip (`if (setlevel) return`) porque los slots ya fueron reservados por `SetMapMonsters()`.
- **`SpawnSkeleton(Player&, Monster&, ...)`**: ubica el esqueleto en el mapa, setea stats (HP, daño, toHit, armor), `MFLAG_GOLEM`, `ai = MonsterAIID::Golem`, envía `NetSendCmdSkeleton` en multiplayer. **Forza tipo `MT_WSKELAX`** con `InitMonster()` si el slot tiene tipo incorrecto (defensa contra deltas antiguos o slot corrupto).
- **`KillMySkeleton()`**: mata el esqueleto propio (reutiliza `CMD_KILLGOLEM` para la sincronización en red)
- **`DeleteMonsterList()`**: resetea esqueletos muertos a `GolemHoldingCell` (mismo patrón que golems)
- **`GolumAi()`** (`Source/monster.cpp:3926-3928`): corregido para calcular correctamente el `ownerId` — si `getId() >= MAX_PLRS`, se resta `MAX_PLRS` para obtener el índice del jugador dueño

### 6. Sistema de misiles (`Source/missiles.cpp`, `Source/missiles.h`)

- **`AddSkeleton()`**: lógica de invocación idéntica a `AddGolem()` pero usando `Monsters[MAX_PLRS + playerId]` en vez de `Monsters[playerId]`
- Declaración `AddSkeleton` agregada en `missiles.h`

### 7. Multiplayer (`Source/msg.cpp`, `Source/msg.h`)

- **`CMD_AWAKESKELETON`**: nuevo comando de red (reutiliza la estructura `TCmdGolem`)
- **`NetSendCmdSkeleton()`**: envía `CMD_AWAKESKELETON` con posición, dirección, enemigo y HP
- **`OnAwakeSkeleton()`**: handler que recibe y crea `MissileID::Skeleton` en el cliente remoto
- **`DeltaSyncSkeleton()`**: guarda estado del esqueleto en `monster[MAX_PLRS + pnum]` del nivel delta
- **`OnKillGolem()`**: ahora también mata el esqueleto (`Monsters[MAX_PLRS + pnum]`) del jugador
- **`DeltaLoadLevel()`**: reconoce esqueletos (`i >= MAX_PLRS && i < MAX_PLRS + MAX_PLRS && monster.type().type == MT_WSKELAX`) como summons tipo golem, llamando `GolumAi()` y setando `MFLAG_GOLEM`. El check de tipo previene aplicar IA de minion a monstruos .dun que pudieran estar en esos slots (defensa contra deltas de saves antiguos).

### 8. Cleanup al cambiar de nivel (`Source/player.cpp`)

- `RemovePlrMissiles()`: mata y limpia el esqueleto además del golem cuando el jugador cambia de nivel

### 9. Fix: GetBookSpell y GetStaffSpell (`Source/items.cpp`)

- **`GetBookSpell()`** (`items.cpp:641`): `maxSpells` era 37 (último spell de Diablo vanilla), pero al mover `Skeleton` a la posición 37, el `while (rv > 0)` con `if (s == maxSpells) s = 1` hacía wrap **antes** de procesar el spell 37, impidiendo que apareciera como libro.
- **Fix:** `maxSpells = 37` → `static_cast<int>(SpellID::LastDiablo) + 1` (= 38), moviendo el wrap a `Mana` (pos 38, `sBookLvl=-1`, nunca elegible).
- **`GetStaffSpell()`** (`items.cpp:1284`): mismo bug con `GetSpellStaffLevel` — fix idéntico.
- Con este cambio, Raise Skeleton es elegible como libro y como enchantment de staff.

### 10. Inicialización (`Source/diablo.cpp`)

- `InitSkeletons()` llamado después de `InitGolems()` en los 3 puntos de carga de nivel (`lvldir == ENTRY_LOAD`, `else`, y set maps). En set levels, `SetMapMonsters()` ya reserva los slots 4-7 antes que los monstruos del `.dun`.

### 10b. Slot reservation en set levels (`Source/monster.cpp`, `Source/msg.cpp`)

Los slots `Monsters[0..3]` (golems) y `Monsters[4..7]` (esqueletos) deben estar siempre reservados para minions, sin importar el tipo de nivel. En niveles normales, `InitGolems()` + `InitSkeletons()` corren antes que `InitMonsters()`, garantizando la reserva. En set levels, el flujo es diferente:

**Problema**: `SetMapMonsters()` (llamada desde `LoadSetMap()`) colocaba monstruos del `.dun` en slots 4-7 antes de que `InitSkeletons()` pudiera reservarlos. Resultado: monstruos del .dun aparecían como "esqueletos" con HP base, y al invocar Raise Skeleton el spell operaba sobre un slot con tipo incorrecto (ej: arquero en vez de MT_WSKELAX).

**Fix**:
- `SetMapMonsters()`: reserva slots 0-3 (golems) + 4-7 (esqueletos con `MT_WSKELAX`) ANTES de procesar la monster layer del `.dun`. Los monstruos del .dun ahora empiezan en slot 8+.
- `InitSkeletons()`: skip en set levels (`if (setlevel) return`) — ya creados por `SetMapMonsters`.
- `SpawnSkeleton(Player&, Monster&, ...)`: fuerza tipo `MT_WSKELAX` con `InitMonster()` si el slot tiene tipo incorrecto.
- `DeltaLoadLevel()` (`msg.cpp`): check `monster.type().type == MT_WSKELAX` antes de aplicar `GolumAi` a slots 4-7.

### 11. MaxSpellLevel: 15 → 4 (`Source/player.h`)

- `MaxSpellLevel` cambiado de 15 a 4 (`player.h:37`), limitando el nivel máximo de todos los hechizos a 4.
- Afecta a Raise Skeleton: `spellLvl` máximo del esqueleto es ahora 4 en vez de 15.
- Impacto en stats del esqueleto a nivel máximo:

| Stat | sl=4 (nuevo max) | sl=15 (anterior max) | Reducción |
|---|---|---|---|
| HP (Mana=200) | 2560 + 2×mana/3 | 9600 + 2×mana/3 | **−73%** |
| ToHit | 60 + 2×playerLevel | 115 + 2×playerLevel | **−48%** |
| Daño min | 4 | 38 | **−89%** |
| Daño max | 10 | 46 | **−78%** |

(Todos los demás hechizos también se ven afectados — ver `docs/spells/spell-tiers.md`.)

## Balance — rol de escudo (sesión 2025-06-13)

El esqueleto está diseñado como **tanque de apoyo**, no como DPS. En nivel 1 hace chip damage (1-4) que no one-shotkea monstruos de Cathedral (2-6 HP), pero tiene HP alto para absorber golpes y distraer enemigos.

| Spell lvl | Daño min | Daño max | HP aprox (50 mana) |
|---|---|---|---|
| 1 | 1 | 4 | ~43 |
| 2 | 2 | 6 | ~53 |
| 3 | 3 | 8 | ~63 |
| 4 | 4 | 10 | ~73 |

## Stats del esqueleto

| Stat | Fórmula |
|---|---|
| HP | `2 × (320 × spellLvl + player._pMaxMana / 3)` |
| ToHit | `5 × (spellLvl + 8) + 2 × player._pLevel` |
| Daño mínimo | `spellLvl` |
| Daño máximo | `2 × spellLvl + 2` |
| Armor Class | 25 |
| Mana cost | 50 |

## Slots de monstruo

| Jugador | Golem (`Monsters[]`) | Esqueleto (`Monsters[]`) |
|---|---|---|
| Player 0 | `[0]` | `[4]` |
| Player 1 | `[1]` | `[5]` |
| Player 2 | `[2]` | `[6]` |
| Player 3 | `[3]` | `[7]` |

## Datos del hechizo

| Campo | Valor |
|---|---|
| SpellID | `Skeleton = 37` |
| MissileID | `Skeleton = 108` |
| Nombre | "Raise Skeleton" |
| Mana cost | 40 |
| Tipo | `Magic \| Targeted` |
| Book level (sBookLvl) | 1 |
| Staff level (sStaffLvl) | 9 |
| Inteligencia mínima (minInt) | 40 |
| Ajuste de mana | 5 |
| Mana mínimo | 20 |
| Staff min | 16 |
| Staff max | 32 |
| Cooldown base (sCooldown) | 1200 ticks (60s) |
| CDR por nivel (sCDR) | 25% |
| Cooldown efectivo | 60s → 45s → 30s → 15s |

## IA

El esqueleto y el golem comparten la misma IA (`GolumAi` en `Source/minion_ai.cpp`) con un sistema de **leash + formación**.

### Sistema de formación

Los minions mantienen una posición de formación relativa al owner, calculada por `GetFormationPosition()`:

- **3 tiles adelante** del owner (en la dirección `owner._pdir`)
- **1 tile lateral**: Golem a la **izquierda** (`Left(dir)`), Esqueleto a la **derecha** (`Right(dir)`)
- La posición base es `owner.position.future` (donde el owner se dirige), no `position.tile` (donde está)

Ejemplo con owner en `(10, 10)` mirando South (forward=(1,1), golem lateral=SouthEast=(1,0), esqueleto lateral=SouthWest=(0,1)):
- Golem target: `(10+3+1, 10+3+0)` = `(14, 13)`
- Esqueleto target: `(10+3+0, 10+3+1)` = `(13, 14)`

### Estados

| Estado | Comportamiento |
|---|---|
| **Normal (Idle)** | Si hay enemigo → Attack (prioridad). Si lejos de formación → Move. Si no, idle mirando a formación. |
| **Move (Follow)** | Scan de enemigos primero (si hay → Attack). Luego se mueve hacia la posición de formación cada tick disponible. Para cuando distToFormation ≤ 2. |
| **Attack (Chase)** | Persigue y ataca al enemigo. Abandona si distToOwner > 6 (leash de combate). |

### Constantes (`Source/minion_ai.cpp`, namespace anónimo)

| Constante | Valor | Descripción |
|---|---|---|
| `MaxMinionReturnDistance` | 4 | Rango de detección de enemigos (ScanForEnemy) |
| `MinionEngageRange` | 3 | Rango desde el owner para considerar un enemigo como "amenaza" |
| `MinionIdleDelay` | 1 | Ticks entre pasos en Move (1 = movimiento continuo) |
| `MinionChaseMaxRange` | 6 | Distancia máxima de persecución antes de rendirse |
| `MinionFollowHysteresis` | 2 | Distancia a formación para dejar de seguir |
| `MinionCombatLeash` | 6 | Distancia máxima del owner durante combate antes de volver |

### Prioridades de decisión

En estado `Normal`: **1° Attack** (ScanForEnemy) → **2° Move** (formación) → **3° Idle**

En estado `Move`: **1° Attack** (ScanForEnemy) → **2° seguir formación**

Esto previene que el minion ignore enemigos mientras sigue al owner.

### Pathfinding

- `AiPlanPathTo(Monster&, Point)` — BFS (`FindPath`) hacia la posición objetivo, con fallback a `RandomWalk`
- `MinionIdleDelay = 1` → el minion se mueve cada tick que no está en animación de caminata
- El minion **nunca usa Teleport** — solo camina, lo que preserva la sincronización en MP

### Transiciones

- `ScanForEnemy()` corre en estados Normal y Move → detección rápida de enemigos
- Al detectar enemigo: transición inmediata a Attack
- Al perder enemigo (muerto, fuera de rango, o leash): vuelve a Normal
- Al abandonar combate (leash): `ClearMinionTarget()` y vuelta a Move

## Visibilidad

### Automapa (Tab)

El esqueleto se muestra en el automapa como una flecha verde:
- Color: `MapColorsMinion = PAL16_BEIGE + 8`
- Solo el esqueleto del jugador local
- Implementado en `DrawAutomapMinion()` en `Source/automap.cpp`

### HUD de estado

El esqueleto tiene un cuadro de estado centrado sobre el panel principal:
- **Layout**: centrado horizontalmente; 1 minion centrado solo, 2 minions lado a lado
- **Contenido**: icono de Raise Skeleton (37x38px) + barra de vida (120x3px) + texto HP
- **Solo visible**: en dungeon y cuando el esqueleto está vivo
- **Color de barra**: verde (>60% HP), amarillo (30-60%), rojo (<30%)
- **Debug**: tag `[STATE]` (FOLLOW/CHASE/IDLE/ATTACK/WALK/DEAD) en builds `_DEBUG`
- Implementado en `DrawMinionStatus()` en `Source/qol/minionstatus.cpp`

## Comportamiento final

| Aspecto | Valor |
|---|---|
| Página del libro | 0 (slot 6) |
| Nombre | Raise Skeleton |
| Mana cost | 50 |
| Tipo | Magic |
| Nivel mínimo de libro (sBookLvl) | 1 |
| Tipo de monstruo | White Skeleton Axe (`MT_WSKELAX`) |
| AI | Golem |
| Slot en Monsters[] | `MAX_PLRS + playerId` |
| Comando de red | `CMD_AWAKESKELETON` |
| Sincronización delta | `monster[MAX_PLRS + pnum]` |
| Holding cell | `GolemHoldingCell (1, 0)` |
| Icono | Frame 24 (Apocalypse) |

## Archivos modificados

- `Source/diablo.cpp` — 1 sección
- `Source/misdat.cpp` — 1 línea
- `Source/missiles.cpp` — 1 función
- `Source/missiles.h` — 1 declaración
- `Source/monster.cpp` — 7 secciones + slot reservation en set levels + idle freeze golem
- `Source/minion_ai.cpp` — IA de minions con formación (3 forward + 1 lateral), leash de combate, ScanForEnemy en Move
- `Source/monster.h` — 3 declaraciones
- `Source/automap.cpp` — `DrawAutomapMinion()` (flecha verde en automapa)
- `Source/qol/minionstatus.cpp` — `DrawMinionStatus()` (HUD icono + barra HP + debug state)
- `Source/qol/minionstatus.h` — declaración de `DrawMinionStatus()`
- `Source/engine/render/scrollrt.cpp` — llamada a `DrawMinionStatus`
- `Source/msg.cpp` — 5 secciones + fix DeltaLoadLevel (type check en slots 4-7)
- `Source/msg.h` — 2 líneas
- `Source/panels/spell_book.cpp` — 2 líneas
- `Source/panels/spell_icons.cpp` — 1 línea
- `Source/player.cpp` — 1 sección
- `Source/items.cpp` — 2 secciones (GetBookSpell, GetStaffSpell)
- `Source/player.h` — 1 línea (MaxSpellLevel)
- `Source/spelldat.cpp` — 1 línea
- `Source/spelldat.h` — 3 líneas
