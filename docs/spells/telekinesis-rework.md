# Telekinesis — Reposicionamiento, knockback múltiple y aturdimiento

## Objetivo

Poner Telekinesis en la primera página del libro de hechizos, y mejorar su efecto contra monstruos: empujar 2 tiles en vez de 1, y aturdir al objetivo ~1 segundo.

## Cambios

### 1. Libro: Telekinesis en página 0 (`Source/panels/spell_book.cpp:36`)

Reemplazado `SpellID::Null` por `SpellID::Telekinesis` en página 0, slot 5 (el slot que quedó vacío al ocultar HealOther).

### 2. Knockback multi-tile (`Source/msg.cpp:1637-1644`)

`OnKnockback` ahora hace un loop de `M_GetKnockback` hasta 2 veces. Si el monstruo está petrificado o contra una pared, se detiene antes.

### 3. Aturdimiento (`Source/msg.cpp:1661-1670`)

Después del knockback, se setea `monster.mode = MonsterMode::Delay` y `monster.flags |= MFLAG_STUNNED`. El monstruo queda congelado en su animación de Stand sin poder actuar.

**Duración:**
- Monstruos normales: **30 ticks** (~1.5s a 20fps)
- Bosses/únicos: **15 ticks** (~0.75s) — 50% reduction

`monster.var2` se usa como countdown. `MonsterDelay()` en `monster.cpp` decrementa `var2` cada tick y restaura `mode = Stand` + limpia `MFLAG_STUNNED` al llegar a 0.

### 4. Fix de dMonster para monstruos caminando (`Source/msg.cpp:1662-1667`)

**Bug:** Si todos los intentos de knockback fallaban (monstruo contra pared), el monstruo seguía en modo walk. Al setear `mode = Delay` sin limpiar `dMonster`, quedaban entradas stale en el tile de destino (sprite duplicado).

**Fix:** Antes de setear `Delay`, si `monster.isWalking()`:
1. `M_ClearSquares(monster)` — limpia ambas entrancias (origen + destino)
2. Snap `position.tile` y `position.future` a `position.old`
3. Escribir entrada limpia: `dMonster[old] = id + 1`

### 5. Indicador visual compartido (`Source/qol/stun_indicator.cpp`)

Telekinesis y Holy Bolt comparten el mismo indicador visual (`MFLAG_STUNNED`). Ver `docs/spells/holybolt-rework.md` para detalles del indicador.

## Comportamiento final

| Aspecto | Valor |
|---|---|
| Página del libro | 0 (slot 5) |
| Nivel mínimo | 2 |
| Knockback | Hasta 2 tiles |
| Aturdimiento (normal) | ~1.5s (30 ticks) |
| Aturdimiento (boss/único) | ~0.75s (15 ticks) |
| Daño | 0 (sin cambios) |
| Abrir cofres/objetos | Sí (sin cambios) |
| Agarrar ítems | Sí (sin cambios) |

## Archivos modificados

- `Source/panels/spell_book.cpp` — 1 línea
- `Source/msg.cpp` — `OnKnockback`: knockback loop, dMonster cleanup, stun con boss reduction, `MFLAG_STUNNED`
