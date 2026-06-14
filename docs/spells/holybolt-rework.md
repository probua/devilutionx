# Holy Bolt — Golpea a todos, daña solo undead, aturde

## Objetivo

Rework de Holy Bolt para darle doble función: sigue siendo el mejor hechizo anti-undead, pero ahora también aturde monstruos vivos sin dañarlos. Esto lo hace útil en cualquier situación.

## Cambios

### 1. Golpea a todos los monstruos (`Source/monster.cpp:~4599`)

Removido el filtro `MonsterClass::Undead` de `isImmune()` para `MissileID::HolyBolt`. El proyectil ahora impacta físicamente contra cualquier monstruo (no lo atraviesa).

### 2. Daño solo a undead, stun a vivos (`Source/missiles.cpp` — `MonsterMHit()`)

Dos caminos dentro de `MonsterMHit`:

**No-undead (stun sin daño):**
- Early return después de `tag()` + sonido de hit
- Aplica stun (ver abajo)
- No calcula ni aplica daño
- Return `true` (el misil impacta y se detiene)

**Undead (daño + stun):**
- Flujo normal de daño (`ApplyMonsterDamage`)
- Si sobrevive, aplica stun después del daño
- Knockback del ítem sigue funcionando si aplica

### 3. Sistema de stun (`Source/missiles.cpp` + `Source/monster.h` + `Source/monster.cpp`)

Usa `MonsterMode::Delay` con `var2` como countdown — mismo patrón que Telekinesis.

| Componente | Ubicación |
|---|---|
| `MFLAG_STUNNED = 1 << 3` | `Source/monster.h:43` |
| Limpieza de flag | `MonsterDelay()` en `Source/monster.cpp:~1527` |
| Seteo de stun (no-undead) | `missiles.cpp:~249-256` |
| Seteo de stun (undead) | `missiles.cpp:~308-315` |

**Duración del stun:**

| Spell lvl | Ticks | Segundos (~20fps) |
|---|---|---|
| 1 | 15 | 0.75s |
| 2 | 20 | 1.0s |
| 3 | 25 | 1.25s |
| 4 (max) | 30 | 1.5s |

Fórmula: `15 + (spllvl - 1) * 5`

**Bosses/únicos:** duración / 2 (50% reduction).

### 4. Fix de dMonster para monstruos caminando (`Source/missiles.cpp`)

**Bug:** Al aturdir un monstruo en movimiento, las entrancias stale de `dMonster` quedaban en el tile de destino. El sprite aparecía duplicado (especialmente con `WalkSideways`, que deja una entrada positiva en el destino).

**Fix:** Antes de setear `MonsterMode::Delay`, si el monstruo está caminando (`isWalking()`):
1. `M_ClearSquares(monster)` — limpia ambas entradas (origen + destino, 3×3 scan)
2. Snap `position.tile` y `position.future` a `position.old` (origen)
3. Escribir entrada limpia: `dMonster[old] = id + 1`

Usar `isWalking()` en vez de `position.tile != position.old` captura los 3 tipos de walk (Northwards, Southwards, Sideways). La condición anterior solo capturaba Southwards.

### 5. Indicador visual de stun (`Source/qol/stun_indicator.h` / `.cpp`)

Tres estrellitas doradas de 5 puntas orbitando sobre la cabeza del monstruo, con efecto de transparencia.

**Características:**
- **3 estrellas** girando a 120° de separación
- **Órbita elíptica isométrica** — eje vertical × 0.5 (simula perspectiva)
- **Estrella de 5 puntas** — 23 píxeles, 7×6 bounding box, array estático de offsets
- **Transparencia checkerboard** — píxeles pares sólidos, impares 50% transparentes (~75% opacidad total)
- **Rotación FPS-independiente** — impulsada por `gGameTicks` (20Hz)
- **Zoom mode** — radio y tamaño de estrella escalados ×2

**Hook:** `DrawView()` en `scrollrt.cpp:~1239`, después de `DrawFloatingNumbers`.

**Pixel layout de la estrella:**
```
Col:     -3  -2  -1   0   1   2   3
Row -3:               X
Row -2:           X   X   X
Row -1:   X   X   X   X   X   X   X
Row  0:       X   X   X   X   X
Row  1:           X   X   X
Row  2:       X   X       X   X
```

## Comportamiento final

| Aspecto | Valor |
|---|---|
| Daño a undead | Sí (sin cambios en fórmula) |
| Daño a no-undead | No (0) |
| Impacto físico | Todos los monstruos |
| Stun a undead | Sí (después de daño) |
| Stun a no-undead | Sí (sin daño) |
| Stun a bosses/únicos | 50% duración |
| Atraviesa monstruos | No (se detiene al impactar) |

## Archivos modificados

| Archivo | Cambio |
|---|---|
| `Source/monster.h` | `MFLAG_STUNNED = 1 << 3` |
| `Source/monster.cpp` | `isImmune()` sin filtro undead para HolyBolt; `MonsterDelay()` limpia `MFLAG_STUNNED` |
| `Source/missiles.cpp` | `MonsterMHit()`: early return no-undead con stun, stun post-daño undead, dMonster cleanup con `isWalking()` |
| `Source/qol/stun_indicator.h` / `.cpp` | Nuevo — indicador visual de estrellas orbitando |
| `Source/engine/render/scrollrt.cpp` | Hook `DrawStunIndicators` en `DrawView` |
| `Source/CMakeLists.txt` | Incluye `qol/stun_indicator.cpp` |
