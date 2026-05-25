# Etapa 6: Generación de mazmorras

## Objetivo

Adaptar los generadores de mazmorra (`drlg_l*.cpp`) para que creen niveles correctos con los nuevos numeros de nivel, incluyendo escaleras, warps de pueblo, y layouts especiales.

## Archivos modificados

### `Source/levels/drlg_l1.cpp` — Cathedral (niveles 1-2)

**Sin cambios necesarios.**

La única referencia a `currlevel` es un `switch` que diferencia los niveles 1 y 2 para el tamaño mínimo del dungeon (`minarea`). Como Cathedral sigue siendo niveles 1-2, funciona correctamente.

```cpp
switch (currlevel) {
case 1: minarea = 533; break;  // Nivel pequeño
case 2: minarea = 693; break;  // Nivel mediano
default: minarea = 761; break; // Nivel estándar
}
```

### `Source/levels/drlg_l2.cpp` — Catacombs (niveles 3-4)

#### `CreateDungeon()` — Tamaño de salas para quests

Originalmente un `switch` con cases 5, 6, 7, 8 determinaba el tamaño de la sala para quest set pieces:

| Case original | Quest | Tamaño | Case mod |
|---|---|---|---|
| 5 | Q_BLOOD | 14x20 | 3 (Q_BLOOD eliminada, `IsAvailable() = false`) |
| 6 | Q_SCHAMB | 10x10 | 4 (Q_SCHAMB eliminada) |
| 7 | Q_BLIND | 15x15 | 4 (Q_BLIND ahora en nivel 4) |
| 8 | (vacío) | — | Eliminado |

Los cases 3 y 4 ahora manejan las quests que quedan. Q_BLOOD y Q_SCHAMB están eliminadas, así que sus checks de `IsAvailable()` siempre dan false. Q_BLIND está en nivel 4.

#### Town warp stairs

Las escaleras de warp al pueblo aparecen en el primer nivel de cada zona. Originalmente `currlevel == 5`.

**Cambio:** `currlevel == 3` (primera Catacombs).

### `Source/levels/drlg_l3.cpp` — Caves (nivel 5)

#### Town warp stairs

**Un solo cambio:** `currlevel == 9` → `currlevel == 5` para las escaleras de warp al pueblo.

Las referencias Hellfire (`currlevel != 17`, `currlevel != 20`) no se tocaron.

### `Source/levels/drlg_l4.cpp` — Hell (niveles 6-7)

Este fue el archivo más complejo de la etapa, con 11 cambios.

#### `LoadQuestSetPieces()` — Set piece de Lazarus

Carga el mapa pre-diseñado para la quest de Lazarus en multiplayer.

**Cambio:** `currlevel == 15` → `currlevel == 6`.

#### `FirstRoom()` — Primera sala del nivel

En el nivel de Diablo (7), la primera sala es siempre fija 14x14. En otros niveles Hell, el tamaño varía.

**Cambios:**
- `currlevel != 16` → `currlevel != 7` (no es Diablo → tamaño variable)
- `currlevel == 16` → `currlevel == 7` (guardar posición de sala Diablo)

#### `PlaceStairs()` — Colocación de escaleras (RESTRUCTURACIÓN CRÍTICA)

Esta fue la función más delicada. El código original:

```
SI currlevel != 15 (no es Lazarus):
    Poner escaleras abajo (si no es nivel 16/Diablo)
    SI currlevel == 13 (primer Hell):
        Poner warp de pueblo
SI NO (es nivel 15/Lazarus):
    Poner puerta del infierno (pentagrama)
```

**El problema:** En el original, el primer nivel de Hell (13) NO era el nivel de Lazarus (15). Pero en el mod, **ambos roles son nivel 6**: primer nivel de Hell (necesita warp) Y nivel de Lazarus (necesita pentagrama).

**Solución:** Nivel 6 hace AMBAS cosas — pentagrama de Lazarus + warp de pueblo:

```
SI currlevel == 6 (Lazarus + primer Hell):
    Poner puerta del infierno (pentagrama)
    Poner warp de pueblo
SI NO SI currlevel != 7:
    Poner escaleras abajo
```

Nivel 7 (Diablo) no tiene escaleras adicionales.

#### `GenerateLevel()` — Generación del nivel

Varias verificaciones dentro del loop de generación:

| Check original | Cambio | Propósito |
|---|---|---|
| `currlevel == 16` | → `7` | Proteger cuadrantes de Diablo |
| `currlevel == 16` | → `7` | Carga inicial de cuadrantes |
| `currlevel != 16` | → `!= 7` | Colocar theme rooms (no en Diablo) |
| `currlevel == 15` | → `6` | Pentagrama/portal Lazarus |
| `currlevel == 16` | → `7` | Carga final de cuadrantes Diablo |

### `Source/levels/themes.cpp` — Salas temáticas

Las salas temáticas son habitaciones especiales generadas aleatoriamente (barriles, santuarios, fosas de monstruos, bibliotecas, etc.). Hay 3 funciones que las manejan, y todas tienen el mismo guard:

```cpp
if (currlevel == 16 || IsAnyOf(leveltype, DTYPE_NEST, DTYPE_CRYPT))
    return;
```

Esto evita generar theme rooms en el nivel de Diablo (que tiene un layout fijo de 4 cuadrantes).

**Cambio:** `currlevel == 16` → `currlevel == 7` en las 3 funciones:
- `InitThemes()` — Inicialización
- `HoldThemeRooms()` — Reservar posiciones
- `CreateThemeRooms()` — Crear objetos

## Consideraciones

- La reestructuración de `PlaceStairs()` fue el cambio más riesgoso de toda la etapa. Verificar en testing que:
  1. Nivel 6 tiene tanto el pentagrama de Lazarus como el warp de pueblo
  2. Nivel 7 tiene los cuadrantes de Diablo correctamente
  3. Las palancas de Diablo funcionan (abren las puertas correctas)
- `drlg_l1.cpp` no necesitó cambios, lo que confirma que los niveles 1-2 de Cathedral son compatibles con el generador original
- Los minarea del switch en `drlg_l1.cpp` podrían ajustarse si los niveles de Cathedral se sienten muy grandes o pequeños — pero el código ya los trata como niveles 1 y 2, que es correcto

## Bug fix: Catacombs quest room sizing

### Problema

Crash al entrar al nivel 4: `bitset::set: __position (which is 1610) >= _Nb (which is 1600)`. La posición 1610 = 40*40 + 10 indica acceso fuera del bitset `Protected[40][40]`.

### Causa

`CreateDungeon()` en `drlg_l2.cpp` usa un switch sobre `currlevel` para asignar tamaños de sala a quests:

```cpp
switch (currlevel) {
case 3:
    if (Quests[Q_BLOOD]._qactive != QUEST_NOTAVAIL)
        size = { 14, 20 };           // Q_BLOOD en nivel 3
    break;
case 4:
    if (Quests[Q_BLIND]._qactive != QUEST_NOTAVAIL)
        size = { 15, 15 };           // Q_BLIND en nivel 4
    break;
}
```

Después del hotfix "Quest reorganization" (Q_BLIND→nivel 3, Q_BLOOD→nivel 4), **este switch no se actualizó**. Resultado:
- Nivel 3: checkea Q_BLOOD (que ya no está en nivel 3) → no crea sala → Q_BLIND no tiene sala
- Nivel 4: checkea Q_BLIND (que ya no está en nivel 4) → no crea sala → Q_BLOOD no tiene sala

Sin sala asignada, el set piece se coloca en coordenadas incorrectas, causando acceso OOB al bitset `Protected`.

### Fix

Actualizado el switch para que coincida con la nueva distribución de quests:

```cpp
switch (currlevel) {
case 3:
    if (Quests[Q_BLIND]._qactive != QUEST_NOTAVAIL)
        size = { 15, 15 };           // Q_BLIND ahora en nivel 3
    break;
case 4:
    if (Quests[Q_BLOOD]._qactive != QUEST_NOTAVAIL)
        size = { 14, 20 };           // Q_BLOOD ahora en nivel 4
    break;
}
```

`LoadQuestSetPieces()` no necesitó cambio — usa `IsAvailable()` que ya verifica `_qlevel == currlevel` dinámicamente.
