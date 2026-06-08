# Hechizos de la Página 1

Página 0 del libro de hechizos: Firebolt, Charged Bolt, Holy Bolt, Healing, Telekinesis, Raise Skeleton.

Todos disponibles desde libro nivel 1 excepto Telekinesis (staff lvl 2) y Raise Skeleton (staff lvl 9).

## Tabla Comparativa

| Hechizo | Mana | Elemento | Daño Mín | Daño Máx | Book Lvl | Staff Lvl |
|---|---|---|---|---|---|---|
| Firebolt | 6 | Fuego | (MAG/8) + sl + 1 | min + 9 | 1 | 1 |
| Charged Bolt | 6 | Rayo | 1 | 1 + (MAG/4) | 1 | 1 |
| Holy Bolt | 7 | Magia | pLvl + 9 | pLvl + 18 | 1 | 1 |
| Healing | 5 | Magia | — | — | 1 | 1 |
| Telekinesis | 15 | Magia | — | — | 1 | 2 |
| Raise Skeleton | 50 | Magia | — | — | 1 | 9 |

**Abreviaturas**: `sl` = nivel de spell (1-4, max 4 en este mod), `MAG` = stat de magia del jugador, `pLvl` = nivel del jugador.

## Detalle por Hechizo

### Firebolt

| Propiedad | Valor |
|---|---|
| Mana | 6 |
| Elemento | Fuego |
| Tipo | Proyectil dirigido |
| Daño | `(MAG/8) + sl + 1` a `(MAG/8) + sl + 10` |
| Velocidad | `16 + min(sl × 2, 47)` (más rápido con más nivel de spell) |
| Efecto secundario | Explosión al impactar (daño menor) |

Daño esperado por nivel de spell (jugador con MAG=30):

| Spell Lvl | Daño | Promedio |
|---|---|---|
| 1 | 5-14 | 9.5 |
| 2 | 6-15 | 10.5 |
| 3 | 7-16 | 11.5 |
| 4 | 8-17 | 12.5 |

Daño esperado por nivel de spell (jugador con MAG=50):

| Spell Lvl | Daño | Promedio |
|---|---|---|
| 1 | 7-16 | 11.5 |
| 2 | 8-17 | 12.5 |
| 3 | 9-18 | 13.5 |
| 4 | 10-19 | 14.5 |

**Inmunidades**: RESIST_FIRE reduce daño. Inefectivo contra Devil Kin, Scavenger, Burning Dead.

### Charged Bolt

| Propiedad | Valor |
|---|---|
| Mana | 6 |
| Elemento | Rayo |
| Tipo | Proyectil con trayectoria errática |
| Daño | 1 a `1 + (MAG/4)` (aleatorio dentro del rango) |
| Comportamiento | Se mueve en patrón zigzag, puede impactar múltiples veces |

Daño esperado por stat de magia:

| MAG | Daño por bolt | Promedio |
|---|---|---|
| 20 | 1-6 | 3.5 |
| 30 | 1-8 | 4.5 |
| 50 | 1-13 | 7 |

**Inmunidades**: RESIST_LIGHTNING reduce daño. Inefectivo contra Plague Eater, Dark One, Horror variants.

### Holy Bolt

| Propiedad | Valor |
|---|---|
| Mana | 7 |
| Elemento | Magia |
| Tipo | Proyectil dirigido |
| Daño | `pLvl + 9` a `pLvl + 18` |
| Restricción | **Solo afecta Undead** (zombies, skeletons, Skeleton King) |

Daño esperado por nivel de jugador:

| pLvl | Daño | Promedio |
|---|---|---|
| 1 | 10-19 | 14.5 |
| 2 | 11-20 | 15.5 |
| 3 | 12-21 | 16.5 |
| 4 | 13-22 | 17.5 |

**Notas**:
- No afecta a monstruos que no sean Undead (no funciona contra Fallen, Scavengers, Bats, Demons)
- Extremadamente efectivo contra el Skeleton King (Undead) si alcanza el nivel 3-4
- El daño escala con nivel de jugador, NO con nivel de spell ni stat de magia

### Healing

| Propiedad | Valor |
|---|---|
| Mana | 5 |
| Elemento | Magia |
| Tipo | Auto-cast (afecta al caster) |
| Se puede usar en pueblo | Sí |

Curación por fórmula de display (`GetDamageAmt`):

| Categoría | Bonus |
|---|---|
| Warrior / Monk / Barbarian | ×2 |
| Rogue / Bard | ×1.5 |
| Sorcerer | ×1 |

Fórmula base (sin bonus de clase):
- Mín: `pLvl + sl + 1`
- Máx: `4 × pLvl + 6 × sl + 10`

Curación esperada por spell level (jugador nivel 2, Guerrero):

| Spell Lvl | Curación | Promedio |
|---|---|---|
| 1 | 7-44 | ~25 |
| 2 | 8-50 | ~29 |
| 3 | 9-56 | ~32 |
| 4 | 10-62 | ~36 |

Curación esperada por spell level (jugador nivel 2, Hechicero):

| Spell Lvl | Curación | Promedio |
|---|---|---|
| 1 | 4-22 | ~13 |
| 2 | 4-25 | ~14 |
| 3 | 5-28 | ~16 |
| 4 | 5-31 | ~18 |

### Telekinesis

| Propiedad | Valor |
|---|---|
| Mana | 15 |
| Elemento | Magia |
| Tipo | Utilidad |
| Daño | Ninguno |
| Efecto | Abre objetos a distancia, recoge items, empuja monstruos |

- No hace daño — es exclusivamente utilidad
- Rework en este mod: ver `docs/spells/telekinesis-rework.md`

### Raise Skeleton

| Propiedad | Valor |
|---|---|
| Mana | 50 |
| Elemento | Magia |
| Tipo | Invocación |
| Max por jugador | 1 esqueleto activo |
| Monster type | White Skeleton Axe (`MT_WSKELAX`) |

Stats del esqueleto invocado:

| Stat | Fórmula |
|---|---|
| HP | `2 × (320 × sl + maxMana / 3)` |
| ToHit | `5 × (sl + 8) + 2 × pLvl` |
| Daño mínimo | `2 × (sl + 4)` |
| Daño máximo | `2 × (sl + 8)` |
| Armor Class | 25 (fijo) |

Stats por spell level (jugador nivel 2, maxMana=100):

| Spell Lvl | HP | ToHit | Daño |
|---|---|---|---|
| 1 | 706 | 55 | 10-18 |
| 2 | 1346 | 60 | 12-20 |
| 3 | 1986 | 65 | 14-22 |
| 4 | 2626 | 70 | 16-24 |

**Notas**:
- El esqueleto usa la misma IA que el Golem (formación lateral, leash de combate)
- No dropea loot, no da XP al morir, pero taggea enemigos para que el jugador reciba XP
- Ver `docs/spells/raise-skeleton.md` para detalles completos

## Guía Rápida de Efectividad por Nivel

### Nivel 1 — qué usar contra qué

| Monstruo | Firebolt | Charged Bolt | Holy Bolt | Notas |
|---|---|---|---|---|
| Zombies | Efectivo | Efectivo | Efectivo | Holy Bolt es la mejor opción (Undead) |
| Fallen | Efectivo | Efectivo | No afecta | No son Undead |
| Skeletons | Efectivo | Efectivo | Efectivo | Holy Bolt es la mejor opción (Undead) |
| Scavengers | Reducido | Efectivo | No afecta | Tienen RESIST_FIRE |
| Fiend (bat) | Efectivo | Efectivo | No afecta | Rápido, difícil de apuntar |
| The Butcher | Reducido | Reducido | No afecta | RESIST_FIRE + RESIST_LIGHTNING |

### Nivel 2 — cambios importantes

| Monstruo | Firebolt | Charged Bolt | Holy Bolt | Notas |
|---|---|---|---|---|
| Black Death | Efectivo | Efectivo | Efectivo | Cuidado: reduce HP máximo |
| Dark One | Efectivo | Reducido | No afecta | RESIST_LIGHTNING |
| Horror variants | Efectivo | Reducido | Efectivo | RESIST_LIGHTNING + IMMUNE_MAGIC |
| Hidden | Efectivo | Efectivo | No afecta | Demon, no Undead |
| Gloom (bat) | Efectivo | Efectivo | No afecta | RESIST_MAGIC, ToHit 70 |
| Skeleton King | Reducido | Reducido | Efectivo | Holy Bolt es la mejor arma contra él |
