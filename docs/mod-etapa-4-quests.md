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

## Quest reorganization (hotfix post-implementación)

### Problema

Q_PWATER no funcionaba en nivel 3 (Catacombs). La entrada al set level (`PWATERIN` miniset) solo es colocada por el generador de Cathedral (`drlg_l1.cpp`), y el generador de Catacombs (`drlg_l2.cpp`) no tiene soporte para PWATER.

### Cambios

| Quest | Antes | Después |
|---|---|---|
| Q_PWATER | Nivel 3 (Catacombs) — no funcionaba | **Desactivada** (QUEST_NOTAVAIL) |
| Q_BLIND | Nivel 4 (Catacombs) | **Nivel 3** (Catacombs) |
| Q_BLOOD | Eliminada | **Nivel 4** (Catacombs) — reactivada |

#### QuestsData[] en `quests.cpp`

- Q_BLIND: `_qdlvl` y `_qdmultlvl` cambiados de 4 a 3
- Q_BLOOD: `_qdlvl` y `_qdmultlvl` cambiados a 4, `isSinglePlayerOnly` cambiado a `false`

#### InitQuests() en `quests.cpp`

Lista de quests eliminadas actualizada: Q_PWATER agregada, Q_BLOOD removida.

### Q_BLOOD soporte en drlg_l2.cpp

Q_BLOOD ya tenía soporte completo en el generador de Catacombs:
- `LoadQuestSetPieces()` carga `blood1.dun`
- Objetos (pedestal, Blood Book) se colocan via `Quests[Q_BLOOD].IsAvailable()`

### Leoric (Q_SKELKING) set level en multiplayer

Originalmente, Leoric solo usaba set level en single-player. En MP aparecía directamente en el nivel.

**Cambios para habilitar set level en MP:**

1. `drlg_l1.cpp` `LoadQuestSetPieces()`: Removido guard `!UseMultiplayerQuests()` para que la tumba (`skngdo.dun`) se coloque también en MP
2. `quests.cpp` `CheckQuests()`: El early return `if (UseMultiplayerQuests()) return;` ahora incluye la lógica de entrada a set levels antes de retornar

**Riesgo:** Mínimo. Leoric es menos complejo que Lazarus (que ya funciona como set level en MP). El set level es estático y el estado se sincroniza via `NetSendCmdQuest()`.

## Set levels fix en multiplayer

### Problema

Dos bugs relacionados con set levels en MP:

1. **Monstruos duplicados:** Leoric aparecía directamente en nivel 2 y Lazarus (+ RedVex + BlackJade) aparecían directamente en nivel 6, en vez de dentro de sus set levels. El jugador los mataba en el nivel principal sin necesidad de entrar a la tumba/Lair.
2. **Re-entrada infinita:** Después de matar a Lazarus, la pentagrama en nivel 6 que debería llevar a nivel 7 (Diablo) enviaba al jugador de vuelta a Lazarus Lair.

### Causa raíz

El juego original tiene dos rutas distintas para quests con set level:

- **SP:** El jefe está dentro del set level. El jugador entra via portal/entrada especial.
- **MP:** El jefe aparece directamente en el nivel principal. No se usa set level.

El mod habilitó los set levels en MP (tumba de Leoric, Lazarus Lair) para que la experiencia sea igual a SP, pero no eliminó el código de colocación directa de monstruos en el nivel principal.

### Cambios en `monster.cpp`

**Leoric (línea ~480):** Eliminado el bloque que colocaba a SkeletonKing directamente en nivel 2 en MP:
```cpp
// ANTES:
if (currlevel == Quests[Q_SKELKING]._qlevel && UseMultiplayerQuests()) {
    for (size_t i = 0; i < LevelMonsterTypeCount; i++) {
        if (IsSkel(LevelMonsterTypes[i].type)) {
            PlaceUniqueMonst(UniqueMonsterType::SkeletonKing, i, 30);
            break;
        }
    }
}

// DESPUÉS: eliminado. Leoric solo se coloca en SL_SKELKING (línea 542)
```

**Lazarus (línea ~517):** Eliminada la colocación directa de Lazarus + RedVex + BlackJade en nivel 6 en MP:
```cpp
// ANTES:
if (currlevel == Quests[Q_BETRAYER]._qlevel && UseMultiplayerQuests()) {
    AddMonsterType(UniqueMonsterType::Lazarus, PLACE_UNIQUE);
    AddMonsterType(UniqueMonsterType::RedVex, PLACE_UNIQUE);
    PlaceUniqueMonst(UniqueMonsterType::Lazarus, 0, 0);
    PlaceUniqueMonst(UniqueMonsterType::RedVex, 0, 0);
    PlaceUniqueMonst(UniqueMonsterType::BlackJade, 0, 0);
    auto dunData = LoadFileInMem<uint16_t>("levels\\l4data\\vile1.dun");
    SetMapMonsters(dunData.get(), SetPiece.position.megaToWorld());
}

// DESPUÉS: solo se mantiene SetMapMonsters para los monstruos del set piece:
if (currlevel == Quests[Q_BETRAYER]._qlevel && UseMultiplayerQuests()) {
    auto dunData = LoadFileInMem<uint16_t>("levels\\l4data\\vile1.dun");
    SetMapMonsters(dunData.get(), SetPiece.position.megaToWorld());
}
```

Lazarus, RedVex y BlackJade solo se colocan dentro de `SL_VILEBETRAYER` (línea 544-550).

### Cambios en `quests.cpp` CheckQuests()

Agregado `quest._qactive != QUEST_DONE` en ambas ramas de detección de set level:

**Rama MP (línea ~299):**
```cpp
if (currlevel == quest._qlevel
    && quest._qslvl != 0
    && quest._qactive != QUEST_NOTAVAIL
    && quest._qactive != QUEST_DONE    // <-- NUEVO
    && MyPlayer->position.tile == quest.position
    && (quest._qidx != Q_BETRAYER || quest._qvar1 >= 3))
```

**Rama SP (línea ~351):** Mismo cambio aplicado.

Esto evita que `CheckQuests()` intercepte al jugador en la pentagrama y lo envíe de vuelta a Lazarus Lair después de completar la quest.

### Flujo correcto después del fix

**Leoric (nivel 2):**
1. Nivel 2 se genera con la tumba (`skngdo.dun`) como set piece
2. Jugador pisa la entrada de la tumba → `CheckQuests()` envía a `SL_SKELKING`
3. Dentro del set level: Leoric se coloca via `PlaceUniqueMonst(SkeletonKing)`
4. Mata a Leoric → quest completa → vuelve a nivel 2 via `WM_DIABRTNLVL`

**Lazarus (nivel 6):**
1. Nivel 6 se genera con el set piece de Lazarus y la pentagrama (`L4PENTA2`)
2. Jugador habla con el altar (`OBJ_ALTBOY`) → activa la quest (`_qvar1 = 3`)
3. Jugador pisa la pentagrama → `CheckQuests()` envía a `SL_VILEBETRAYER`
4. Dentro del set level: Lazarus + RedVex + BlackJade se colocan via `PlaceUniqueMonst()`
5. Mata a Lazarus → `_qactive = QUEST_DONE`, `_qvar1 = 7`, Q_DIABLO se activa
6. Al morir Lazarus en MP: se buscan pentagramas (tile 369) y se registran como `WM_DIABNEXTLVL`
7. Vuelve a nivel 6 → la pentagrama ahora tiene `WM_DIABNEXTLVL` (no `WM_DIABSETLVL`)
8. Pisa la pentagrama → `CheckTriggers()` envía a nivel 7 (Diablo)
9. `CheckQuests()` ya no intercepta porque `_qactive == QUEST_DONE`

## Lazarus quest flow completo (Staff + Cain + portal rojo)

### Problema

Después del fix anterior, Lazarus no aparecía en ningún lado en MP — se eliminó su colocación directa en nivel 6, pero no se implementó un mecanismo alternativo para entrar al set level. El portal rojo nunca aparecía porque la quest se auto-activaba en MP (`_qvar1 = 2` en `InitQuests()`), sin seguir el flujo original del Staff of Lazarus.

### Flujo original de Diablo (SP)

1. Nivel 14-15: Jugador encuentra el **Staff of Lazarus** (`IDI_LAZSTAFF`) en un pedestal (`OBJ_LAZSTAND`)
2. Pueblo: Jugador habla con **Cain** (storyteller) con el Staff → Cain lo toma → quest activada (`_qvar1 = 2`, `_qactive = QUEST_ACTIVE`)
3. Nivel 15: `CheckQuests()` SP detecta `_qvar1 >= 2` → genera **portal rojo** en la pentagrama
4. Jugador pisa el portal → entra a `SL_VILEBETRAYER` (Lazarus Lair)
5. Dentro: Lazarus habla → se vuelve hostil → combate → muerte
6. Portal rojo dentro del set level → vuelve a nivel 15
7. Pentagrama lleva a nivel 16 (Diablo)

### Flujo original de Diablo (MP)

En MP, Lazarus no usa set level. Se coloca directamente en nivel 15 con `_qvar1 = 2`. El jugador habla con Lazarus (NPC hostil), se vuelve agresivo, lo mata, y la pentagrama se activa para ir a nivel 16.

### Flujo del mod (SP y MP — unificado)

El mod usa el flujo SP para ambos modos. La quest se activa con Staff + Cain en ambos casos.

1. **Nivel 6**: Se genera el **pedestal del Staff** (`OBJ_LAZSTAND`) — `AddLazStand()` ahora funciona en MP también
2. Jugador interactúa con el pedestal → obtiene el **Staff of Lazarus**
3. **Pueblo**: Jugador habla con **Cain** → Cain toma el Staff → `_qactive = QUEST_ACTIVE`, `_qvar1 = 2` → `NetSendCmdQuest()` sincroniza para todos los jugadores en MP
4. **Nivel 6**: `CheckQuests()` detecta `_qactive == QUEST_ACTIVE && _qvar2 == 0` → genera **portal rojo** en la pentagrama
5. Jugador pisa el portal rojo → `CheckQuests()` envía a `SL_VILEBETRAYER`
6. Dentro: Lazarus se coloca via `PlaceUniqueMonst()`. `_qvar1 <= 3` → Lazarus inicia diálogo (`LazarusAi()` rama MP, línea 2799)
7. Habla con Lazarus → `MonsterTalk()` → `_qvar1 = 6` → Lazarus se vuelve hostil
8. Mata a Lazarus → `_qactive = QUEST_DONE`, `_qvar1 = 7`. Se buscan pentagramas (tile 369) → `WM_DIABNEXTLVL`
9. Portal rojo dentro del set level → vuelve a nivel 6
10. Pentagrama tiene `WM_DIABNEXTLVL` → lleva a nivel 7 (Diablo)
11. `CheckQuests()` no intercepta porque `_qactive == QUEST_DONE`

### Cambios por archivo

#### `objects.cpp` (~línea 4044)

```cpp
// ANTES:
if (Quests[Q_BETRAYER].IsAvailable() && !UseMultiplayerQuests())
    AddLazStand();

// DESPUÉS:
if (Quests[Q_BETRAYER].IsAvailable())
    AddLazStand();
```

El pedestal del Staff ahora se genera en MP también.

#### `quests.cpp` InitQuests() (~línea 269)

```cpp
// ANTES:
if (UseMultiplayerQuests())
    Quests[Q_BETRAYER]._qvar1 = 2;

// DESPUÉS: eliminado. _qvar1 empieza en 0 (default).
```

La quest no se pre-activa en MP — necesita el Staff + Cain.

#### `quests.cpp` ResyncMPQuests() (~línea 565)

```cpp
// ANTES:
auto &betrayerQuest = Quests[Q_BETRAYER];
if (betrayerQuest._qactive == QUEST_INIT && currlevel == betrayerQuest._qlevel - 1) {
    betrayerQuest._qactive = QUEST_ACTIVE;
    NetSendCmdQuest(true, betrayerQuest);
}
if (betrayerQuest.IsAvailable())
    AddObject(OBJ_ALTBOY, SetPiece.position.megaToWorld() + Displacement { 4, 6 });

// DESPUÉS: eliminado completamente. La quest se activa solo via Cain + Staff.
```

- Auto-activate al pasar por nivel 5: eliminado
- Altar decorativo (`OBJ_ALTBOY`) en nivel 6: eliminado — el altar no tiene función en el nuevo flujo

#### `quests.cpp` CheckQuests() (~línea 287)

```cpp
// ANTES:
if (quest.IsAvailable() && UseMultiplayerQuests() && quest._qvar1 == 2) {
    AddObject(OBJ_ALTBOY, ...);
    quest._qvar1 = 3;
    NetSendCmdQuest(true, quest);
}

// DESPUÉS: eliminado. El altar ya no se coloca en nivel 6.
```

El portal rojo se genera con el código que ya existía (rama MP, `_qactive == QUEST_ACTIVE && _qvar2 == 0`).

#### `towners.cpp` TalkToStoryteller() (~línea 554)

```cpp
// ANTES (rama MP):
} else {
    if (betrayerQuest._qactive == QUEST_ACTIVE && !betrayerQuest._qlog) {
        InitQTextMsg(TEXT_VILE1);
        betrayerQuest._qlog = true;
        NetSendCmdQuest(true, betrayerQuest);
        return;
    }
}

// DESPUÉS (rama MP):
} else {
    if (betrayerQuest._qactive == QUEST_INIT && RemoveInventoryItemById(player, IDI_LAZSTAFF)) {
        InitQTextMsg(TEXT_VILE1);
        betrayerQuest._qlog = true;
        betrayerQuest._qactive = QUEST_ACTIVE;
        betrayerQuest._qvar1 = 2;
        NetSendCmdQuest(true, betrayerQuest);
        return;
    }
    if (betrayerQuest._qactive == QUEST_ACTIVE && !betrayerQuest._qlog) {
        InitQTextMsg(TEXT_VILE1);
        betrayerQuest._qlog = true;
        NetSendCmdQuest(true, betrayerQuest);
        return;
    }
}
```

Ahora Cain acepta el Staff en MP (igual que SP) y activa la quest. `NetSendCmdQuest()` sincroniza el estado para todos los jugadores.

### Sincronización en multiplayer

| Evento | Mecanismo | Sincronizado para todos? |
|---|---|---|
| Staff en pedestal | `SpawnQuestItem()` → `CMD_DELTAITEMS` | Sí |
| Agarrar Staff | Item pickup estándar | Un jugador lo tiene |
| Hablar con Cain | `NetSendCmdQuest()` | Sí — quest activada para todos |
| Portal rojo | `AddMissile(RedPortal)` → `CMD_SPAWNMISSILE` | Sí |
| Entrar al set level | `StartNewLvl(WM_DIABSETLVL)` | Individual (cada jugador entra por separado) |
| Lazarus diálogo | `NetSendCmdQuest()` | Sí |
| Matar a Lazarus | `NetSendCmdQuest()` | Sí |
| Volver + pentagrama | `InitL4Triggers()` local | Sí (mismos tiles para todos) |
