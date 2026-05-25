# Bug fix: Flash Spell — Misil permanente

## Problema

El hechizo Flash (`AddFlashBottom`) tenía dos efectos visuales:

1. **Sprite cortado/incompleto**: La animación de Flash se veía parcial, como si faltara la mitad del efecto.
2. **Invulnerabilidad eterna**: Después de lanzar Flash, el jugador se volvía permanentemente invulnerable al daño de los monstruos.

## Causa raíz

`AddFlashBottom()` en `missiles.cpp` no inicializaba `_mirange` (el número de frames que dura el misil). Sin inicialización, `_mirange` tenía un valor indeterminado (basura del stack o 0).

Cuando `_mirange` es 0 o un valor muy alto:
- El misil nunca se elimina del juego (se considera "activo" permanentemente)
- El sistema de colisión de misiles detecta Flash como un escudo permanente alrededor del jugador
- Los monstruos no pueden dañar al jugador porque el misil Flash "absorbe" el contacto

## Fix (commit `fe3b864d2`)

```cpp
// missiles.cpp, AddFlashBottom() línea 2057
missile._midam = std::max(missile._midam, 1);
missile._mirange = 19;  // <-- AGREGADO
```

`_mirange = 19` coincide con el valor que usa `AddFlashTop()` (la mitad superior del efecto). Flash dura 19 frames (~0.6 segundos) y luego se elimina correctamente.

## Nota

Este es un **bug de devilutionX original**, no introducido por el mod. El `_mirange` faltante probablemente se perdió durante un refactor del código de misiles. El hechizo es funcionalmente idéntico en Diablo original, donde el valor se inicializaba correctamente.

## Archivos modificados

| Archivo | Línea | Cambio |
|---|---|---|
| `Source/missiles.cpp` | 2057 | Agregado `missile._mirange = 19` en `AddFlashBottom()` |
