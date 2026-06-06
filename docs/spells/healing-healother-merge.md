# Healing + Heal Other — Fusión en libro de hechizos

## Objetivo

Unificar Healing y Heal Other en una sola entrada del libro de hechizos, aprendiendo ambos al leer un libro de Healing, y eliminando Heal Other como libro dropeable o como enchantment de staff.

## Cambios

### 1. Libro: ocultar HealOther (`Source/panels/spell_book.cpp:36`)

Reemplazado `SpellID::HealOther` por `SpellID::Null` en la página 0, slot 5. Healing ocupa el slot 4 (sin cambios).

### 2. Libros: aprender ambos al leer (`Source/items.cpp:4129-4136`)

Al leer un libro de Healing, después de subir el nivel de Healing, se envía también `CMD_CHANGE_SPELL_LEVEL` para HealOther al mismo nivel.

### 3. Libros: no dropear HealOther (`Source/items.cpp:661-662`)

`GetBookSpell()` ahora siempre salta HealOther en la selección aleatoria de libros (antes solo en single player).

### 4. Staffs: no generar HealOther (`Source/items.cpp:1304-1305`)

`GetStaffSpell()` ahora siempre salta HealOther como posible enchantment (antes solo en single player).

## Comportamiento final

| Aspecto | Healing | Heal Other |
|---|---|---|
| Libro (página 0) | Visible | Oculto |
| Aprendido al leer libro de Healing | Sí | Sí (automático) |
| Dropea como libro | Sí | No |
| Dropea como scroll | Sí | Sí (sin cambios) |
| Aparece como enchantment de staff | Sí | No |
| Spell bar / hotkeys | Sí (Spell o Charges) | Sí (Spell) |
| Healer shop | — | Solo scroll en multiplayer (sin cambios) |

## Archivos modificados

- `Source/panels/spell_book.cpp` — 1 línea
- `Source/items.cpp` — 3 secciones (`GetBookSpell`, `GetStaffSpell`, `UseItem` `IMISC_BOOK`)
