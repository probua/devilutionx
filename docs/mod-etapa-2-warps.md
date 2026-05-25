# Etapa 2: Warps y triggers

## Objetivo

Configurar las transiciones entre niveles: escaleras, portales town warp, y triggers especiales (cada vez que el jugador baja/sube un nivel, cruza a una zona nueva, o usa un portal).

## Archivos modificados

### `Source/levels/trigs.cpp` — Triggers de transición

Contiene toda la lógica de escaleras y warps.

#### `ForceL4Trig()` — Triggers de Hell

Esta función busca tiles especiales en el mapa de Hell y registra sus posiciones como triggers (escaleras, warps, pentagramas).

Originalmente verificaba `currlevel == 15` (Lazarus) y `currlevel == 16` (Diablo). Cambiado a `currlevel == 6` y `currlevel == 7`.

No hay conflicto entre el warp de town (nivel 6) y el pentagrama de Diablo (nivel 7) porque detectan tiles diferentes (`L4TWarpUpList` vs `L4PentaList`).

#### `InitTownWarps()` — Warps del pueblo

Coloca los portales de warp en el pueblo que llevan a cada zona. Originalmente los warps se activaban al llegar a niveles 5, 9 y 13.

**Cambio:** Los warps se activan al llegar a niveles 3, 5 y 6:

```
Nivel 3 (Catacombs)  → warp activado en pueblo
Nivel 5 (Caves)      → warp activado en pueblo
Nivel 6 (Hell)       → warp activado en pueblo
```

#### `TWarpFrom()` — Posiciones de spawn

Cuando el jugador usa un warp del pueblo para teletransportarse a una zona, esta función determina dónde aparece. Las posiciones de spawn están en `town.cpp`.

### `Source/levels/town.cpp` — Spawn positions

Contiene las posiciones de los warps en el pueblo. Cambios menores para alinear con los nuevos niveles.

### `Source/interfac.cpp` — Cutscenes

Controla qué cinematica se reproduce al cambiar de zona.

Originalmente:
- Nivel 16 = cinematica de Diablo (`MOVHOME` / `INTRO`)

**Cambio:** Nivel 7 = cinematica de Diablo.

## Flujo de transición

El flujo de un jugador que baja por toda la mazmorra:

```
Town → Nivel 1 (Cathedral)
     → Nivel 2 (Cathedral) [warp town activado para Cathedral]
     → Nivel 3 (Catacombs) [warp town activado para Catacombs]
     → Nivel 4 (Catacombs)
     → Nivel 5 (Caves) [warp town activado para Caves]
     → Nivel 6 (Hell) [warp town activado para Hell]
              └→ Portal Lazarus (set level SL_VILEBETRAYER)
     → Nivel 7 (Diablo) [cinematica final]
```

## Consideraciones

- Lazarus es un **set level** (mapa pre-diseñado) accesible desde nivel 6 via portal. No es un `currlevel` propio.
- Los set levels (`SL_SKELKING`, `SL_BONECHAMB`, `SL_POISONWATER`, `SL_VILEBETRAYER`) usan `setlevel = true` y `setlvlnum` cuando están activos, separados del flujo de niveles normales.
- Los town warps permiten al jugador volver a zonas anteriores sin tener que caminar todo de nuevo.
