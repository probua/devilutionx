# Monstruos del Nivel 1

Mod level 1 = Cathedral. Virtual level = 1 (mapeo original).

## Monstruos Regulares (19)

### Melee - Zombies

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Zombie | 4-7 | 2-5 | 5 | 10 | Undead | IMMUNE_MAGIC | 54 |
| Ghoul | 7-11 | 3-10 | 10 | 10 | Undead | IMMUNE_MAGIC | 58 |
| Rotting Carcass | 15-25 | 5-15 | 15 | 25 | Undead | IMMUNE_MAGIC | 136 |

### Melee - Fallen (pueden huir y volver en grupo)

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Fallen One (spear) | 1-4 | 1-3 | 0 | 15 | Animal | — | 46 |
| Carver (spear) | 4-8 | 2-5 | 5 | 20 | Animal | — | 80 |
| Devil Kin (spear) | 12-24 | 3-7 | 10 | 25 | Animal | RESIST_FIRE | 155 |
| Fallen One (sword) | 2-5 | 1-4 | 10 | 15 | Animal | — | 52 |
| Carver (sword) | 5-9 | 2-7 | 15 | 20 | Animal | — | 90 |
| Devil Kin (sword) | 16-24 | 4-10 | 20 | 25 | Animal | RESIST_FIRE | 180 |

### Melee - Skeletons

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Skeleton (axe) | 2-4 | 1-4 | 0 | 20 | Undead | IMMUNE_MAGIC | 64 |
| Corpse Axe | 4-7 | 3-5 | 0 | 25 | Undead | IMMUNE_MAGIC | 68 |
| Burning Dead (axe) | 8-12 | 3-7 | 5 | 30 | Undead | IMMUNE_MAGIC, RESIST_FIRE | 154 |
| Skeleton Captain | 3-6 | 2-7 | 10 | 20 | Undead | IMMUNE_MAGIC | 90 |
| Corpse Captain | 12-20 | 3-9 | 5 | 30 | Undead | IMMUNE_MAGIC | 200 |

### Melee - Scavengers (pueden comer cadáveres para curarse)

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Scavenger | 3-6 | 1-5 | 10 | 20 | Animal | RESIST_FIRE | 80 |
| Plague Eater | 12-24 | 1-8 | 20 | 30 | Animal | RESIST_LIGHTNING | 188 |

### Ranged - Skeletons con arco

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Skeleton (bow) | 2-4 | 1-2 | 0 | 15 | Undead | IMMUNE_MAGIC | 110 |
| Corpse Bow | 8-16 | 1-4 | 0 | 25 | Undead | IMMUNE_MAGIC | 210 |

### Volador

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| Fiend (bat) | 3-6 | 1-6 | 0 | 35 | Animal | — | 102 |

## Boss de Quest

| Monstruo | HP | Daño | AC | ToHit | Clase | Resistencias | Exp |
|---|---|---|---|---|---|---|---|
| **The Butcher** | 320 | 6-12 | 50 | 50 | Demon | RESIST_FIRE, RESIST_LIGHTNING | 710 |

- Quest: The Butcher (se activa al entrar a nivel 1)
- Aparece en una habitación cerrada
- Velocidad de ataque alta, persigue al jugador constantemente

## Resumen para Balanceo

### Rangos de HP por categoría

| Categoría | HP | Ejemplos |
|---|---|---|
| Muy débil | 1-6 | Fallen One, Skeleton (axe), Fiend, Zombie, Scavenger |
| Medio | 7-16 | Ghoul, Carver, Corpse Axe, Skeleton (bow) |
| Fuerte | 16-25 | Devil Kin, Rotting Carcass, Plague Eater, Corpse Captain |
| Boss | 320 | The Butcher |

### Rangos de daño por categoría

| Categoría | Daño | Ejemplos |
|---|---|---|
| Bajo | 1-5 | Fallen One, Skeletons, Scavenger |
| Medio | 3-10 | Ghoul, Carver, Corpse Axe |
| Alto | 5-15 | Rotting Carcass, Devil Kin, Corpse Captain |
| Boss | 6-12 | The Butcher (pero con ToHit 50 y AC 50) |

### Resistencias

- **Todos los Undead** (Zombies, Skeletons): IMMUNE_MAGIC — hechizos de magia pura no funcionan
- **Devil Kin**: RESIST_FIRE
- **Scavenger**: RESIST_FIRE
- **Plague Eater**: RESIST_LIGHTNING
- **The Butcher**: RESIST_FIRE + RESIST_LIGHTNING — solo daño físico o mágico puro

### NPCs únicos que pueden aparecer

| Nombre | Base | HP | Daño | Resistencias |
|---|---|---|---|---|
| Soulpus | Zombie | 133 | 4-8 | RESIST_FIRE, RESIST_LIGHTNING |
| Pukerat the Unclean | Fallen One (spear) | 77 | 1-5 | RESIST_FIRE |
| Boneripper | Skeleton (axe) | 54 | 6-15 | IMMUNE_MAGIC, IMMUNE_FIRE |
| Rotfeast the Hungry | Zombie | 85 | 4-12 | IMMUNE_MAGIC |
| Shadowbite | Scavenger | 60 | 3-20 | IMMUNE_FIRE |
| Deadeye | Skeleton (bow) | 49 | 6-9 | IMMUNE_MAGIC, RESIST_FIRE |
