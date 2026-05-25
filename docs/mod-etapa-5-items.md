# Etapa 5: Items y objetos

## Objetivo

Adaptar el sistema de generación de items para que funcione correctamente con 7 niveles, y arreglar los objetos especiales de mazmorra (story books, Diablo levers, Slain Hero).

## Archivos modificados

### `Source/items.cpp` — `ItemsGetCurrlevel()`

Esta es la función central del sistema de items. Devuelve un "nivel de items" que se usa para:

- Filtrar items por calidad (`iMinMLvl`) en drops, tiendas y hechizos
- Calcular bonus de items (`GetItemBonus` usa `lvl * 2` como maxLevel)
- Determinar qué libros de hechizos pueden aparecer

Originalmente devolvía `currlevel` directamente (1-16). Ahora mapea a niveles virtuales.

**Cambio:** Cuando `setlevel == false` y no es Hellfire, retorna `GetVirtualLevel()` en vez de `currlevel`.

Para set levels (SL_SKELKING, SL_POISONWATER, SL_VILEBETRAYER), obtiene el `_qlevel` de la quest y lo mapea con `GetVirtualLevelForShop()`.

#### Cómo funciona la generación de items

La fórmula clave en el juego es:

```cpp
itemMaxLevel = ItemsGetCurrlevel() * 2;
```

Con `currlevel` real (1-7), el maxLevel seria 2-14, perdiendo todos los items de Hell (niveles 15-16). Con `GetVirtualLevel()`:

| Nivel real | ItemsGetCurrlevel() | maxLevel (x2) | Items disponibles |
|---|---|---|---|
| 1 | 1 | 2 | Cathedral básicos |
| 2 | 3 | 6 | Cathedral completos |
| 3 | 5 | 10 | Catacombs |
| 4 | 7 | 14 | Catacombs completos |
| 5 | 9 | 18 | Caves (todos los items) |
| 6 | 13 | 26 | Hell |
| 7 | 16 | 32 | Todos los items del juego |

#### Por qué no modificar `itemdat.cpp`

La alternativa era modificar `iMinMLvl` en `AllItemsList[]` (la tabla de datos de items). Problemas:

- 100+ items con valores hardcoded
- Habría que recalcular cada `iMinMLvl` para la nueva escala
- Los valores de `PLMinLvl` (prefixes) y `UIMinLvl` (unique items) también necesitarían cambio
- Un solo error rompe la progresión de items

La solución de mapeo virtual es más simple y centralizada: un solo cambio en `ItemsGetCurrlevel()` en vez de cientos de cambios en tablas de datos.

### `Source/objects.cpp` — Objetos de mazmorra

#### Story Books (libros de historia)

Libros grandes en el suelo que el héroe lee al hacer clic. Contienen texto de lore. Aparecen en el último piso de cada tema.

Originalmente: niveles 4, 8, 12 (último piso de Cathedral, Catacombs, Caves).  
**Cambio:** niveles 2, 4, 6 (último piso de Cathedral, Catacombs, Hell).

La función `AddStoryBook()` determina qué texto mostrar según el nivel. Originalmente usaba `currlevel / 4` para calcular el set de texto. Ahora usa un mapeo explícito:

```cpp
if (currlevel == 2)      storySet = 0;  // Set 1 (Cathedral)
else if (currlevel == 4) storySet = 1;  // Set 2 (Catacombs)
else if (currlevel == 6) storySet = 2;  // Set 3 (Hell)
```

Hay 3 sets de 3 textos cada uno (9 textos totales en el juego).

#### AddDiabObjs — Objetos del nivel de Diablo

Palancas y pentagrama del nivel final. Originalmente `currlevel == 16`.

**Cambio:** `currlevel == 7`.

#### UpdateLeverState / SyncLever — Palancas de Diablo

Verifican que todas las palancas estén activadas antes de abrir las puertas del nivel de Diablo.

**Cambio:** `currlevel == 16` → `currlevel == 7` en ambas funciones.

#### Slain Hero — Héroe caído

Cadáver de un guerrero que da un item mágico al hacer clic. Originalmente `currlevel == 9` (primer nivel de Caves).

**Cambio:** `currlevel == 5` (Caves en el mod).

## Objetos de quest (sin cambios adicionales)

Estos objetos se generan basándose en `Quest.IsAvailable()`, que ya usa `quest._qlevel` (remapeado en Etapa 4):

| Objeto | Quest | Control |
|---|---|---|
| Sala de tortura | Q_BUTCHER | `IsAvailable()` → nivel 1 |
| Velas | Q_PWATER | `IsAvailable()` → nivel 3 |
| Libro ciego | Q_BLIND | `IsAvailable()` → nivel 4 |
| Yunque | Q_ANVIL | `IsAvailable()` → nivel 5 |
| Book of Vileness | Q_BETRAYER | `IsAvailable()` → nivel 6 |

## Bug fix: InitObjectGFX() storybook graphics

### Problema

Al cambiar `AddStoryBooks()` de niveles 4,8,12 a niveles 2,4,6 en la Etapa 5, se omitió actualizar `InitObjectGFX()` que carga los gráficos de storybook. Esta función tiene un check hardcoded:

```cpp
if (IsAnyOf(currlevel, 4, 8, 12)) {
    filesWidths[OFILE_BKSLBRNT] = AllObjects[OBJ_STORYBOOK].animWidth;
    filesWidths[OFILE_CANDLE2] = AllObjects[OBJ_STORYCANDLE].animWidth;
}
```

Solo cargaba los gráficos de storybook para los niveles originales. Al entrar al nivel 2 (Cathedral), se intentaba colocar un `OBJ_STORYBOOK` cuyo gráfico `OFILE_BKSLBRNT` (id 53) nunca se cargó, causando un crash:

```
CRITICAL: Unable to find object_graphic_id 53 in list of objects to load, level generation error.
Segmentation fault (core dumped)
```

### Fix

`objects.cpp` línea ~3817: `IsAnyOf(currlevel, 4, 8, 12)` → `IsAnyOf(currlevel, 2, 4, 6)`.

### Lección

Cuando se cambian los niveles donde se colocan objetos, hay que verificar **dos** cosas:
1. **Colocación** (`AddStoryBooks()`, `InitObjects()`) — dónde se crean los objetos
2. **Carga de recursos** (`InitObjectGFX()`) — dónde se cargan los gráficos necesarios

Ambas deben estar sincronizadas.

## Bug fix: InitObjectGFX() level-range loading

### Problema

Además del fix de storybooks, `InitObjectGFX()` tiene un loop que carga gráficos según `minlvl`/`maxlvl` de cada objeto en `AllObjects[]`. Estos rangos están basados en niveles originales (1-16):

```cpp
if (objectData.minlvl != 0 && currlevel >= objectData.minlvl && currlevel <= objectData.maxlvl)
```

Objetos de Catacombs tienen `minlvl=5, maxlvl=8`. Con `currlevel=3` (nuestro primer nivel de Catacombs), la condición `3 >= 5` es false — los gráficos de antorchas (OFILE_WTORCH1-4, IDs 25, 26, 32, 33) no se cargaban, causando crash al entrar al nivel 3.

Lo mismo ocurriría con objetos de Caves (minlvl=9) y Hell (minlvl=13) en sus niveles del mod.

### Fix

`objects.cpp` línea ~3823: `currlevel` → `GetVirtualLevel()` en la comparación de rangos:

```cpp
if (objectData.minlvl != 0 && GetVirtualLevel() >= objectData.minlvl && GetVirtualLevel() <= objectData.maxlvl)
```

Ahora el mapeo es consistente:
- Nivel 3 (Catacombs) → `GetVirtualLevel() = 5` → antorchas y objetos de Catacombs cargados
- Nivel 5 (Caves) → `GetVirtualLevel() = 9` → objetos de Caves cargados
- Nivel 6-7 (Hell) → `GetVirtualLevel() = 13-16` → objetos de Hell cargados

### Nota

Este fix puede cargar algunos gráficos extra (ej: objetos de niveles 13-16 en nivel 6 cuando el generador no los coloca), pero esto solo consume un poco más de memoria y no causa bugs. Es el mismo enfoque usado en todo el mod con `GetVirtualLevel()`.
