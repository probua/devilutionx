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
- **`InitSkeletons()`**: pre-crea esqueletos en `GolemHoldingCell` para cada jugador (slots `MAX_PLRS + playerId`)
- **`SpawnSkeleton()`**: ubica el esqueleto en el mapa, setea stats (HP, daño, toHit, armor), `MFLAG_GOLEM`, `ai = MonsterAIID::Golem`, envía `NetSendCmdSkeleton` en multiplayer
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
- **`DeltaLoadLevel()`**: reconoce esqueletos (`i >= MAX_PLRS`) como summons tipo golem, llamando `GolumAi()` y setando `MFLAG_GOLEM`

### 8. Cleanup al cambiar de nivel (`Source/player.cpp`)

- `RemovePlrMissiles()`: mata y limpia el esqueleto además del golem cuando el jugador cambia de nivel

### 9. Fix: GetBookSpell y GetStaffSpell (`Source/items.cpp`)

- **`GetBookSpell()`** (`items.cpp:641`): `maxSpells` era 37 (último spell de Diablo vanilla), pero al mover `Skeleton` a la posición 37, el `while (rv > 0)` con `if (s == maxSpells) s = 1` hacía wrap **antes** de procesar el spell 37, impidiendo que apareciera como libro.
- **Fix:** `maxSpells = 37` → `static_cast<int>(SpellID::LastDiablo) + 1` (= 38), moviendo el wrap a `Mana` (pos 38, `sBookLvl=-1`, nunca elegible).
- **`GetStaffSpell()`** (`items.cpp:1284`): mismo bug con `GetSpellStaffLevel` — fix idéntico.
- Con este cambio, Raise Skeleton es elegible como libro y como enchantment de staff.

### 10. Inicialización (`Source/diablo.cpp`)

- `InitSkeletons()` llamado después de `InitGolems()` en los 3 puntos de carga de nivel (`lvldir == ENTRY_LOAD`, `else`, y set maps)

### 11. MaxSpellLevel: 15 → 4 (`Source/player.h`)

- `MaxSpellLevel` cambiado de 15 a 4 (`player.h:37`), limitando el nivel máximo de todos los hechizos a 4.
- Afecta a Raise Skeleton: `spellLvl` máximo del esqueleto es ahora 4 en vez de 15.
- Impacto en stats del esqueleto a nivel máximo:

| Stat | sl=4 (nuevo max) | sl=15 (anterior max) | Reducción |
|---|---|---|---|
| HP (Mana=200) | 2560 + 2×mana/3 | 9600 + 2×mana/3 | **−73%** |
| ToHit | 60 + 2×playerLevel | 115 + 2×playerLevel | **−48%** |
| Daño min | 16 | 38 | **−58%** |
| Daño max | 24 | 46 | **−48%** |

(Todos los demás hechizos también se ven afectados — ver `docs/spells/spell-tiers.md`.)

## Stats del esqueleto

| Stat | Fórmula |
|---|---|
| HP | `2 × (320 × spellLvl + player._pMaxMana / 3)` |
| ToHit | `5 × (spellLvl + 8) + 2 × player._pLevel` |
| Daño mínimo | `2 × (spellLvl + 4)` |
| Daño máximo | `2 × (spellLvl + 8)` |
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
| Mana cost | 50 |
| Tipo | `Magic \| Targeted` |
| Book level (sBookLvl) | 1 |
| Staff level (sStaffLvl) | 9 |
| Inteligencia mínima (minInt) | 40 |
| Ajuste de mana | 6 |
| Mana mínimo | 60 |
| Staff min | 16 |
| Staff max | 32 |

## IA

El esqueleto usa la misma IA que el golem (`GolumAi`) con un sistema de **leash** (correa) que controla el comportamiento según la distancia al dueño y la presencia de enemigos:

### Comportamiento por distancia (sin enemigo cercano)

| Dist. al dueño | Estado | Comportamiento |
|---|---|---|
| 0-4 | **Idle** | Si jugador camina: sigue con pathfind+delay. Si jugador quieto: se queda quieto mirando al dueño |
| 5-8 | **Relajado** | Camina hacia dueño con pathfinding, 1 paso cada 4 ticks (`MinionIdleDelay`) |
| >8 | **Urgente** | Pathfind directo hacia dueño, cada tick |

### Comportamiento con enemigo (a <= `MinionEngageRange` = 5 tiles)

| Dist. al dueño | Comportamiento |
|---|---|
| <= `MaxMinionChaseDistance` (4) | Perseguir enemigo con `AiPlanPath`, atacar si adyacente (`StartAttack`) |
| > 4 | Volver al dueño, ataque solo si enemigo adyacente |

### Seguimiento del jugador

- Cuando el jugador **ataca** (`PM_ATTACK` o `PM_RATTACK`), el minion avanza en la dirección `_pdir` del jugador usando `RandomWalk`, con un delay de `MinionIdleDelay` ticks
- Cuando el jugador **camina** (se mueve sin atacar), el minion sigue usando `AiPlanPathTo` con delay
- El minion **nunca usa Teleport** — solo camina, lo que preserva la sincronización en MP

### Pathfinding

- `AiPlanPathTo(Monster&, Point)` — BFS (`FindPath`) hacia la posición del dueño, con fallback a `RandomWalk` directo si no encuentra ruta
- `var2` se usa como contador de delay entre pasos (safe porque `GolumAi` no lo usaba antes)

### Transiciones

- `UpdateEnemy()` corre cada tick → detección de enemigos es instantánea (~50ms)
- Al detectar enemigo: `var2 = 0`, modo activo inmediato
- Al perder enemigo: transición a idle/relajado en el siguiente tick

### Constantes definidas en `Source/monster.cpp` (namespace anónimo)

| Constante | Valor | Descripción |
|---|---|---|
| `MaxMinionChaseDistance` | 4 | Radio libre para perseguir enemigos |
| `MaxMinionReturnDistance` | 8 | Distancia de retorno urgente |
| `MinionEngageRange` | 5 | Rango para activar modo combate |
| `MinionIdleDelay` | 4 | Ticks entre pasos en modo relajado |

## Visibilidad

### Automapa (Tab)

El esqueleto se muestra en el automapa como una flecha verde:
- Color: `MapColorsMinion = PAL16_BEIGE + 8`
- Solo el esqueleto del jugador local
- Implementado en `DrawAutomapMinion()` en `Source/automap.cpp`

### HUD de estado

El esqueleto tiene un cuadro de estado en la esquina inferior izquierda del viewport:
- **Posición**: arriba a la izquierda del panel principal (`panel.x + 4`)
- **Contenido**: icono de Raise Skeleton (37x38px) + barra de vida (30x3px) + texto HP
- **Solo visible**: en dungeon y cuando el esqueleto está vivo
- **Color de barra**: verde (>60% HP), amarillo (30-60%), rojo (<30%)
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
- `Source/monster.cpp` — 7 secciones + IA leash (3 estados, pathfinding, follow)
- `Source/monster.h` — 3 declaraciones
- `Source/automap.cpp` — `DrawAutomapMinion()` (flecha verde en automapa)
- `Source/qol/minionstatus.cpp` — `DrawMinionStatus()` (HUD icono + barra HP)
- `Source/qol/minionstatus.h` — declaración de `DrawMinionStatus()`
- `Source/engine/render/scrollrt.cpp` — llamada a `DrawMinionStatus`
- `Source/msg.cpp` — 5 secciones
- `Source/msg.h` — 2 líneas
- `Source/panels/spell_book.cpp` — 2 líneas
- `Source/panels/spell_icons.cpp` — 1 línea
- `Source/player.cpp` — 1 sección
- `Source/items.cpp` — 2 secciones (GetBookSpell, GetStaffSpell)
- `Source/player.h` — 1 línea (MaxSpellLevel)
- `Source/spelldat.cpp` — 1 línea
- `Source/spelldat.h` — 3 líneas
