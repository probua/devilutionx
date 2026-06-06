# Telekinesis — Reposicionamiento, knockback múltiple y aturdimiento

## Objetivo

Poner Telekinesis en la primera página del libro de hechizos, y mejorar su efecto contra monstruos: empujar 2 tiles en vez de 1, y aturdir al objetivo ~1 segundo.

## Cambios

### 1. Libro: Telekinesis en página 0 (`Source/panels/spell_book.cpp:36`)

Reemplazado `SpellID::Null` por `SpellID::Telekinesis` en página 0, slot 5 (el slot que quedó vacío al ocultar HealOther).

### 2. Knockback multi-tile (`Source/msg.cpp:1637-1644`)

`OnKnockback` ahora hace un loop de `M_GetKnockback` hasta 2 veces. Si el monstruo está petrificado o contra una pared, se detiene antes.

### 3. Aturdimiento (`Source/msg.cpp:1647-1648`)

Después del knockback, se setea `monster.var2 = 30` y `monster.mode = MonsterMode::Delay`. El monstruo queda congelado en su animación de Stand ~1 segundo (30 ticks a 30fps) sin poder actuar.

## Comportamiento final

| Aspecto | Valor |
|---|---|
| Página del libro | 0 (slot 5) |
| Nivel mínimo | 2 |
| Knockback | Hasta 2 tiles |
| Aturdimiento | ~1 segundo |
| Daño | 0 (sin cambios) |
| Abrir cofres/objetos | Sí (sin cambios) |
| Agarrar ítems | Sí (sin cambios) |

## Archivos modificados

- `Source/panels/spell_book.cpp` — 1 línea
- `Source/msg.cpp` — `OnKnockback` (~8 líneas agregadas)
