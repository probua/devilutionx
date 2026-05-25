# Etapa 4: Quests

## Objetivo

Remapear las 7 quests activas a los niveles correctos del mod, eliminar 9 quests innecesarias, y simplificar los pools de quests aleatorias.

## Archivos modificados

### `Source/quests.cpp` — Datos y lógica de quests

#### `QuestsData[]` — Tabla de quests

Cada quest tiene un `_qlevel` (nivel de mazmorra donde aparece) y `_qlvltype` (tipo de mazmorra).

**Cambios de nivel:**

| Quest | _qlevel original | _qlevel mod | _qlvltype |
|---|---|---|---|
| Q_BUTCHER | 2 | 1 | DTYPE_CATHEDRAL |
| Q_SKELKING | 1 | 2 | DTYPE_CATHEDRAL |
| Q_PWATER | 5 | 3 | DTYPE_CATACOMBS |
| Q_BLIND | 7 | 4 | DTYPE_CATACOMBS |
| Q_ANVIL | 9 | 5 | DTYPE_CAVES |
| Q_BETRAYER | 15 | 6 | DTYPE_HELL |
| Q_DIABLO | 16 | 7 | DTYPE_HELL |

#### Quests habilitadas para multiplayer

Originalmente, algunas quests eran single-player only (`isSinglePlayerOnly = true`). Se cambiaron a disponibles en ambos modos:

- **Q_PWATER** (Poisoned Water) — Ahora disponible en MP
- **Q_BLIND** (Halls of the Blind) — Ahora disponible en MP
- **Q_ANVIL** (Anvil of Fury) — Ahora disponible en MP

Para cada una se agregó `_qdmultlvl` (mensaje de nivel multiplayer).

#### `InitQuests()` — Inicialización

Después de que los pools de quests se procesan, se fuerza `QUEST_NOTAVAIL` para las 9 quests eliminadas:

```cpp
Quests[Q_ROCK]._qactive = QUEST_NOTAVAIL;
Quests[Q_MUSHROOM]._qactive = QUEST_NOTAVAIL;
Quests[Q_GARBUD]._qactive = QUEST_NOTAVAIL;
Quests[Q_ZHAR]._qactive = QUEST_NOTAVAIL;
Quests[Q_VEIL]._qactive = QUEST_NOTAVAIL;
Quests[Q_LTBANNER]._qactive = QUEST_NOTAVAIL;
Quests[Q_BLOOD]._qactive = QUEST_NOTAVAIL;
Quests[Q_WARLORD]._qactive = QUEST_NOTAVAIL;
Quests[Q_SCHAMB]._qactive = QUEST_NOTAVAIL;
```

#### `InitialiseQuestPools()` — Simplificación

Los pools originales (`QuestGroup1` a `QuestGroup4`) determinaban qué quests aleatorias se activaban en cada nivel. Con 9 quests eliminadas, la mayoría de los pools quedaron vacíos.

**Cambio:** Se eliminaron los arrays `QuestGroup1-4` y se simplificó la función para solo mantener la randomización entre Q_SKELKING y Q_PWATER.

#### `glSeedTbl` — Semillas de generación

Las semillas para generación de niveles se indexaban con los números de nivel originales:

- `glSeedTbl[15]` → `glSeedTbl[6]` (Lazarus/Hell)
- `glSeedTbl[16]` → `glSeedTbl[7]` (Diablo)

### `Source/towners.cpp` — NPCs del pueblo

Los NPCs del pueblo tienen dialogo condicional basado en `_pLvlVisited[quest._qlevel]` — verifican si el jugador ya visitó el nivel de la quest.

Originalmente usaban indices hardcoded como `_pLvlVisited[5]`, `_pLvlVisited[7]`, etc.

**Cambio:** Se reemplazaron los indices hardcoded por `quest._qlevel` dinámico para Q_ANVIL, Q_SKELKING, y Q_PWATER.

### `Source/objects.cpp` — Objetos de quest

- `glSeedTbl[16]` → `glSeedTbl[7]` para generación de objetos

### `Source/control.cpp` — Debug/display

- `glSeedTbl[16]` → `glSeedTbl[7]` para display de debug

## Riesgo de desync en multiplayer

Algunas quests que antes eran SP-only ahora funcionan en MP. El riesgo de desync (los clientes ven estados diferentes) varía:

| Quest | Riesgo | Razón |
|---|---|---|
| PWATER | Medio-alto | Verifica cantidad de monstruos en set level |
| BLIND | Medio | Verifica estado de objetos |
| ANVIL | Medio | Verifica estado de objetos |

Todas usan `NetSendCmdQuest()` para sincronizar estado, lo que mitiga el riesgo.

## Consideraciones

- Las quests eliminadas se fuerzan a `QUEST_NOTAVAIL` **después** de que los pools se procesan. Esto es seguro porque los pools ya no las incluyen (están vacíos).
- Poisoned Water (Q_PWATER) se colocó en Catacombs (nivel 3). Aunque es una quest "de agua", el set level `SL_POISONWATER` usa tiles `DTYPE_CAVES` internamente — no hay conflicto visual.
- Las referencias a `_pLvlVisited` para quests eliminadas (Q_LTBANNER, Q_ROCK) en `towners.cpp` son inofensivas — están protegidas por `_qactive != QUEST_NOTAVAIL`.
- Las referencias Hellfire (Q_FARMER, Q_JERSEY) en `towners.cpp` con `_pLvlVisited[9]` y `_pLvlVisited[21]` no necesitan cambio.
