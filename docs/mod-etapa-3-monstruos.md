# Etapa 3: Monstruos

## Objetivo

Remapear los 93 monstruos del juego para que aparezcan en los niveles correctos del mod (1-7) en vez de los niveles originales (1-16).

## Archivos modificados

### `Source/monstdat.cpp` — Datos de monstruos

Contiene la tabla `MonstersData[]` con todos los monstruos del juego. Cada monstruo tiene:

- `minDunLvl` — Nivel mínimo de mazmorra donde aparece
- `maxDunLvl` — Nivel máximo de mazmorra donde aparece

**Cambio:** Todos los `minDunLvl` y `maxDunLvl` fueron remapeados usando un script de Python.

#### Regla de mapeo

La distribución original era 4+4+4+4 niveles por tema. El mod usa 2+2+1+2. El mapeo por posición relativa dentro de cada tema:

| Posición en tema | Original (4 niveles) | Mod (2 niveles) | Mod (1 nivel) |
|---|---|---|---|
| 1er piso | 1, 5, 9, 13 | 1, 3, 5, 6 | — |
| 2do piso | 2, 6, 10, 14 | 2, 4, 5, 7 | — |
| 3er piso | 3, 7, 11, 15 | — | — |
| 4to piso | 4, 8, 12, 16 | — | — |

Para monstruos que abarcan múltiples niveles originales, se mapearon al rango equivalente comprimido.

#### Script de remapeo

Se uso un script Python que:
1. Leía cada monstruo de `MonstersData[]`
2. Mapeaba `minDunLvl` y `maxDunLvl` según la zona
3. Ajustaba para que no hubiera gaps ni superposiciones incorrectas
4. Generaba el código C++ de reemplazo

### `Source/monster.cpp` — Lógica de monstruos

#### `GetLevelMTypes()` — Tipos de monstruos por nivel

Determina qué tipos de monstruos pueden aparecer en cada nivel. Originalmente verificaba `currlevel == 16` para colocar al boss Diablo.

**Cambio:** `currlevel == 7` para colocar a Diablo.

#### `InitMonsters()` — Inicialización

Controla la cantidad de monstruos y checks especiales. Originalmente verificaba `currlevel == 16` y `currlevel == 15`.

**Cambios:**
- `currlevel == 16` → `currlevel == 7` (Diablo)
- `currlevel == 15` → `currlevel == 6` (Lazarus)

#### `IsMonsterAvalible()` — Sin cambio

Esta función usa `currlevel >= minDunLvl && currlevel <= maxDunLvl` — automáticamente funciona con los datos remapeados en `monstdat.cpp`.

## Monstruos notables

| Monstruo | Original | Mod | Nota |
|---|---|---|---|
| Diablo (boss) | Nivel 16 | Nivel 7 | Hardcodeado en `GetLevelMTypes()` |
| The Butcher | Nivel 2 | Nivel 1 | Quest Q_BUTCHER |
| Skeleton King | Nivel 1 | Nivel 2 | Quest Q_SKELKING (set level) |
| Stormwing (min boss) | Nivel 5 | Nivel 3 | Catacombs |
| Flame Wheel | Nivel 9 | Nivel 5 | Caves |
| Lich | Nivel 13 | Nivel 6 | Hell |

## Consideraciones

- Los monstruos de Hellfire (niveles 17-24) no se modificaron
- La función `IsMonsterAvalible()` usa los valores `minDunLvl`/`maxDunLvl` directamente, así que el remapeo de datos es suficiente
- Los niveles de los monstruos (`mLevel`) no se cambiaron — solo los niveles de mazmorra donde aparecen
- El script de Python permitió remapear los 93 monstruos de forma consistente sin errores manuales
