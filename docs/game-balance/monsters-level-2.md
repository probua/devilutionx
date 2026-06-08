# Monstruos del Nivel 2

Mod level 2 = Cathedral. Virtual level = 3 (mapeo original).

Salto significativo de poder respecto al nivel 1. Los monstruos nuevos tienen HP hasta 50, daño hasta 22, y AC hasta 70.

## Monstruos Heredados del Nivel 1 (15)

Los siguientes monstruos del nivel 1 siguen apareciendo (tienen maxDunLvl >= 2):

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Ghoul | 7-11 | 3-10 | 10 | 10 | Undead | IMMUNE_MAGIC | 58 |
| Rotting Carcass | 15-25 | 5-15 | 15 | 25 | Undead | IMMUNE_MAGIC | 136 |
| Carver (spear) | 4-8 | 2-5 | 5 | 20 | Animal | — | 80 |
| Devil Kin (spear) | 12-24 | 3-7 | 10 | 25 | Animal | RESIST_FIRE | 155 |
| Corpse Axe | 4-7 | 3-5 | 0 | 25 | Undead | IMMUNE_MAGIC | 68 |
| Burning Dead (axe) | 8-12 | 3-7 | 5 | 30 | Undead | IMMUNE_MAGIC, RESIST_FIRE | 154 |
| Carver (sword) | 5-9 | 2-7 | 15 | 20 | Animal | — | 90 |
| Devil Kin (sword) | 16-24 | 4-10 | 20 | 25 | Animal | RESIST_FIRE | 180 |
| Scavenger | 3-6 | 1-5 | 10 | 20 | Animal | RESIST_FIRE | 80 |
| Plague Eater | 12-24 | 1-8 | 20 | 30 | Animal | RESIST_LIGHTNING | 188 |
| Skeleton (bow) | 2-4 | 1-2 | 0 | 15 | Undead | IMMUNE_MAGIC | 110 |
| Corpse Bow | 8-16 | 1-4 | 0 | 25 | Undead | IMMUNE_MAGIC | 210 |
| Skeleton Captain | 3-6 | 2-7 | 10 | 20 | Undead | IMMUNE_MAGIC | 90 |
| Corpse Captain | 12-20 | 3-9 | 5 | 30 | Undead | IMMUNE_MAGIC | 200 |
| Fiend (bat) | 3-6 | 1-6 | 0 | 35 | Animal | — | 102 |

## Monstruos Nuevos del Nivel 2 (13)

### Melee - Zombies

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Black Death | 25-40 | 6-22 | 20 | 30 | Undead | IMMUNE_MAGIC | 240 |

- Efecto especial: reduce永久mente el HP máximo del jugador al golpear

### Melee - Fallen

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Dark One (spear) | 20-36 | 4-8 | 15 | 30 | Animal | RESIST_LIGHTNING | 255 |
| Dark One (sword) | 24-36 | 4-12 | 25 | 30 | Animal | RESIST_LIGHTNING | 280 |

### Melee - Skeletons

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Horror (axe) | 12-20 | 4-9 | 15 | 35 | Undead | IMMUNE_MAGIC, RESIST_LIGHTNING | 264 |
| Burning Dead Captain | 16-30 | 4-10 | 15 | 35 | Undead | IMMUNE_MAGIC, RESIST_FIRE | 393 |
| Horror Captain | 35-50 | 5-14 | 30 | 40 | Undead | IMMUNE_MAGIC, RESIST_LIGHTNING | 604 |

- Horror Captain: tiene MFLAG_SEARCH (busca al jugador activamente)

### Melee - Scavengers

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Shadow Beast | 24-36 | 3-12 | 25 | 35 | Animal | RESIST_FIRE | 375 |
| Bone Gasher | 28-40 | 5-15 | 30 | 35 | Animal | RESIST_MAGIC | 552 |

### Ranged - Skeletons con arco

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Burning Dead (bow) | 10-24 | 1-6 | 5 | 30 | Undead | IMMUNE_MAGIC, RESIST_FIRE | 364 |
| Horror (bow) | 15-45 | 2-9 | 15 | 35 | Undead | IMMUNE_MAGIC, RESIST_LIGHTNING | 594 |

### Invisible

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Hidden | 8-24 | 3-6 | 25 | 35 | Demon | — | 278 |

- Se hace invisible, ataca por sorpresa, luego se hace invisible otra vez

### Voladores

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Blink (bat) | 12-28 | 1-8 | 15 | 45 | Animal | — | 340 |
| Gloom (bat) | 28-36 | 4-12 | 35 | 70 | Animal | RESIST_MAGIC | 509 |

- Gloom: ToHit 70 + MFLAG_SEARCH — extremadamente agresivo y preciso

## Boss de Quest

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| **Skeleton King** | 140 | 6-16 | 70 | 60 | Undead | IMMUNE_MAGIC, RESIST_FIRE, RESIST_LIGHTNING | 570 |

- Quest: Leoric (se activa en nivel 2)
- Teletransporta al jugador a su cámara al activar la quest
- Invoca esqueletos adicionales
- AC 70 muy alta — difícil de impactar con ataques físicos al inicio del juego
- Unique data: mMaxHP=240 (sobreescribe el 140 base), mLevel=0 (usa base + 5 = nivel 14)

## Resumen para Balanceo

### Rangos de HP por categoría

| Categoría | HP | Ejemplos |
|---|---|---|
| Débil (heredados) | 3-12 | Carver, Scavenger, Corpse Axe, Fiend |
| Medio | 12-30 | Devil Kin, Horror (axe), Burning Dead Captain, Hidden |
| Fuerte | 28-50 | Bone Gasher, Shadow Beast, Horror Captain, Black Death |
| Boss | 140-240 | Skeleton King (240 HP efectivos) |

### Rangos de daño por categoría

| Categoría | Daño | Ejemplos |
|---|---|---|
| Bajo | 1-8 | Carver, Skeletons, Scavenger, Blink |
| Medio | 4-12 | Dark One, Devil Kin, Shadow Beast, Gloom |
| Alto | 5-22 | Horror Captain, Bone Gasher, Black Death |
| Boss | 6-16 | Skeleton King |

### Comparación Nivel 1 vs Nivel 2

| Métrica | Nivel 1 | Nivel 2 (nuevos) | Incremento |
|---|---|---|---|
| HP máximo (regular) | 25 | 50 | ×2 |
| Daño máximo (regular) | 15 | 22 | ×1.5 |
| AC máxima (regular) | 20 | 35 | ×1.75 |
| ToHit máxima (regular) | 35 | 70 | ×2 |
| HP Boss | 320 | 240 | −25% (peroSkeleton King invoca adds) |

### Resistencias

- **Todos los Undead**: IMMUNE_MAGIC (sigue siendo la mayoría)
- **Dark One**: RESIST_LIGHTNING (cambio respecto a Devil Kin que era RESIST_FIRE)
- **Bone Gasher**: RESIST_MAGIC (poco común)
- **Horror variants**: RESIST_LIGHTNING (además de IMMUNE_MAGIC)
- **Skeleton King**: IMMUNE_MAGIC + RESIST_FIRE + RESIST_LIGHTNING — solo daño físico efectivo

### NPCs únicos que pueden aparecer (adicionales a los del nivel 1)

| Nombre | Base | HP | Daño | Resistencias |
|---|---|---|---|---|
| Bonehead Keenaxe | Corpse Axe | 91 | 4-10 | IMMUNE_MAGIC |
| Bladeskin the Slasher | Fallen One (sword) | 51 | 6-18 | RESIST_FIRE |
| Gutshank the Quick | Carver (sword) | 66 | 6-16 | RESIST_FIRE |
| Brokenhead Bangshield | Corpse Captain | 108 | 12-20 | IMMUNE_MAGIC, RESIST_LIGHTNING |
| Bongo | Devil Kin (spear) | 178 | 9-21 | — |
| Rotcarnage | Ghoul | 102 | 9-24 | IMMUNE_MAGIC, RESIST_LIGHTNING |
| El Chupacabras | Plague Eater | 120 | 10-18 | RESIST_FIRE |
| Skullfire | Corpse Bow | 125 | 6-10 | IMMUNE_FIRE |
| Warpskull | Hidden | 117 | 6-18 | RESIST_FIRE, RESIST_LIGHTNING |
| Goretongue | Rotting Carcass | 156 | 15-30 | IMMUNE_MAGIC |
| Madeye the Dead | Burning Dead (axe) | 75 | 9-21 | IMMUNE_MAGIC, IMMUNE_FIRE |
