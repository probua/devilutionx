# Etapa 1: Niveles y temas

## Objetivo

Definir la estructura de la mazmorra de 7 niveles: qué tema (Cathedral, Catacombs, Caves, Hell) corresponde a cada nivel, y cuántos niveles tiene el juego en total.

## Archivos modificados

### `Source/levels/gendung.cpp` — `GetLevelType()`

Función que devuelve el tipo de mazmorra (theme) para cada nivel. Originalmente mapeaba niveles 1-4 a Cathedral, 5-8 a Catacombs, 9-12 a Caves, 13-16 a Hell.

**Cambio:** Remapeo a la distribución 2+2+1+2:

```cpp
dungeon_type GetLevelType()
{
    if (setlevel)
        return setlvltype;

    if (currlevel >= 1 && currlevel <= 2)
        return DTYPE_CATHEDRAL;
    if (currlevel >= 3 && currlevel <= 4)
        return DTYPE_CATACOMBS;
    if (currlevel == 5)
        return DTYPE_CAVES;
    if (currlevel >= 6 && currlevel <= 7)
        return DTYPE_HELL;

    // ... Hellfire levels (Nest, Crypt) sin cambio
}
```

### `Source/diablo.cpp` — `giNumberOfLevels`

Variable que controla cuántos niveles tiene el juego. Originalmente era 17 (Diablo) o 25 (Hellfire).

**Cambio:** `giNumberOfLevels = 8` (7 niveles de mazmorra + town).

Esto se setea en `StartGame()` (linea ~2394). El valor 8 incluye el nivel 0 (town) más los niveles 1-7.

## Constante `NUMLEVELS` — Sin cambio

`NUMLEVELS` se mantiene en 25 (definido en `diablo.h`). Razones:

1. Muchos arrays en el juego están dimensionados con `NUMLEVELS` (25):
   - `glSeedTbl[NUMLEVELS]` — semillas de generación
   - `Player::_pLvlVisited[NUMLEVELS]` — niveles visitados
   - `Player::_pSLvlVisited[NUMLEVELS]` — set levels visitados
2. El código de Hellfire (niveles 17-24) referencia estos arrays con indices hasta 24
3. Reducir `NUMLEVELS` causaría buffer overflows en paths de código Hellfire

`giNumberOfLevels` controla la lógica de gameplay (cuántos niveles jugar), mientras `NUMLEVELS` controla el tamaño de los arrays de soporte.

## Consideraciones

- `GetLevelType()` es la función central — muchos sistemas dependen de ella indirectamente (generación de mazmorra, monstruos disponibles, tiles gráficos)
- La distribución 2+2+1+2 significa que Caves tiene solo 1 nivel. Esto es aceptable y agrega variedad al gameplay — el jugador no se queda estancado mucho tiempo en Caves
- Los niveles de Hellfire (17-24) quedan intactos. Si `gbIsHellfire == false`, nunca se acceden
