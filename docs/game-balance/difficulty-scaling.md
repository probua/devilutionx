# Escalado de Monstruos por Dificultad

## Multiplicadores por Dificultad

Los monstruos escalan al inicializarse según la dificultad elegida (Normal, Nightmare, Hell).

### HP

| Dificultad | Fórmula | Notas |
|---|---|---|
| Normal | `random(hpMin, hpMax)` | Tal cual de la tabla de datos |
| Nightmare | `Normal × 3 + 100` | Triplicado más bonus plano |
| Hell | `Normal × 4 + 200` | Cuadruplicado más bonus plano |

En Singleplayer, todos los HP se dividen por 2 (mínimo 1).

### Daño (ataque normal y especial)

| Dificultad | minDamage | maxDamage |
|---|---|---|
| Normal | `data.minDamage` | `data.maxDamage` |
| Nightmare | `2 × (data.minDamage + 2)` | `2 × (data.maxDamage + 2)` |
| Hell | `4 × data.minDamage + 6` | `4 × data.maxDamage + 6` |

### Armor Class

| Dificultad | armorClass |
|---|---|
| Normal | `data.armorClass` |
| Nightmare | `data.armorClass + 50` |
| Hell | `data.armorClass + 80` |

### ToHit (chance de golpear al jugador)

| Dificultad | toHit |
|---|---|
| Normal | `data.toHit` |
| Nightmare | `data.toHit + 85` |
| Hell | `data.toHit + 120` |

### Resistencias

| Dificultad | resistance |
|---|---|
| Normal | `data.resistance` |
| Nightmare | `data.resistance` (igual que Normal) |
| Hell | `data.resistanceHell` (tabla separada, generalmente más resistencias) |

En Hell, muchos monstruos ganan inmunidades adicionales. Por ejemplo, Burning Dead (axe) pasa de RESIST_FIRE a IMMUNE_FIRE.

### Nivel del monstruo (usado en fórmulas de combate)

| Dificultad | nivel |
|---|---|
| Normal | `data.level` |
| Nightmare | `data.level + 15` |
| Hell | `data.level + 30` |

Para uniques: si `mlevel != 0`, se usa `mlevel × 2`. Si `mlevel == 0`, se usa `data.level + 5`. Luego se suma el bonus de dificultad.

### Experiencia otorgada

| Dificultad | Regular | Unique |
|---|---|---|
| Normal | `data.exp` | `data.exp × 2` |
| Nightmare | `2 × (data.exp + 1000)` | `2 × (data.exp + 1000) × 2` |
| Hell | `4 × (data.exp + 1000)` | `4 × (data.exp + 1000) × 2` |

## Ejemplos Concretos

### Zombie (HP 4-7, daño 2-5, AC 5, ToHit 10, nivel 1)

| Stat | Normal | Nightmare | Hell |
|---|---|---|---|
| HP (SP) | 2-3 | 106-109 | 416-421 |
| HP (MP) | 4-7 | 112-121 | 836-849 |
| Daño | 2-5 | 8-14 | 14-26 |
| AC | 5 | 55 | 85 |
| ToHit | 10 | 95 | 130 |
| Nivel | 1 | 16 | 31 |
| Exp | 54 | 2108 | 4216 |

### Devil Kin sword (HP 16-24, daño 4-10, AC 20, ToHit 25, nivel 5)

| Stat | Normal | Nightmare | Hell |
|---|---|---|---|
| HP (SP) | 8-12 | 148-172 | 668-716 |
| HP (MP) | 16-24 | 448-520 | 1344-1504 |
| Daño | 4-10 | 12-24 | 22-46 |
| AC | 20 | 70 | 100 |
| ToHit | 25 | 110 | 145 |
| Nivel | 5 | 20 | 35 |
| Exp | 180 | 2360 | 4720 |

### The Butcher (HP 320, daño 6-12, AC 50, ToHit 50, nivel 1)

| Stat | Normal | Nightmare | Hell |
|---|---|---|---|
| HP (SP) | 160 | 1640 | 6560 |
| HP (MP) | 320 | 3280 | 13120 |
| Daño | 6-12 | 16-28 | 30-54 |
| AC | 50 | 100 | 130 |
| ToHit | 50 | 135 | 170 |
| Nivel | 1 | 16 | 31 |
| Exp | 710 | 3420 | 6840 |

## Fórmula de Acierto del Monstruo

Cuando un monstruo ataca al jugador, la chance de impactar se calcula así:

```
chance = ToHit_monstruo + 2 × (nivelMonstruo - nivelJugador) + 30 - armaduraJugador
```

El resultado se compara con `random(0, 99)`. Si `random < chance`, el ataque impacta.

### Qué significa cada componente

- **ToHit del monstruo**: base del dato + bonus por dificultad (+85/+120)
- **2 × (nivelMonstruo - nivelJugador)**: diferencia de niveles amplificada ×2. Si el monstruo es de mayor nivel, golpea más. Si el jugador es de mayor nivel, el monstruo golpea menos
- **30**: base fija (sin esto, la chance sería muy baja)
- **armaduraJugador**: reduce la chance de ser golpeado. Incluye armadura base + bonus por equipo

### Ejemplo 1: Zombie vs Guerrero nivel 1 con AC 10

```
chance = 10 + 2 × (1 - 1) + 30 - 10 = 30%
```

El Zombie tiene 30% de chance de pegarte. Bastante manejable.

### Ejemplo 2: Gloom (bat) vs Guerrero nivel 2 con AC 15

```
chance = 70 + 2 × (9 - 2) + 30 - 15 = 109%
```

El Gloom te pega prácticamente siempre. ToHit 70 es brutal para nivel 2.

### Ejemplo 3: Skeleton King vs Jugador nivel 3 con AC 30

```
chance = 60 + 2 × (14 - 3) + 30 - 30 = 82%
```

El Skeleton King tiene 82% de golpearte. Su AC 70 también hace que sea difícil de impactar.

### Ejemplo 4: Mismo Gloom pero en Nightmare vs Guerrero nivel 2 con AC 15

```
chance = (70 + 85) + 2 × (24 - 2) + 30 - 15 = 199%
```

Capping en ~100%. En Nightmare los monstruos golpean casi siempre.

## Bonus para Minions de Unique

Los monstruos que acompañan a un boss unique tienen **HP × 2** (se aplica después del escalado por dificultad).

Ejemplo: un minion de Skeleton King en Normal (SP):
- Base: Corpse Captain (HP 12-20)
- Singleplayer: 6-10
- Como minion: 12-20

## Regeneración de HP del Monstruo

Los monstruos regeneran HP lentamente cuando están activos:

```
HP += nivelMonstruo       (cada tick, monstruo normal)
HP += nivelMonstruo / 2   (cada tick, si es Gargoyle)
```

Esto significa que en Nightmare/Hell, donde los niveles son más altos (+15/+30), los monstruos regeneran mucho más rápido. Un monstruo de nivel 30 en Hell regenera 30 HP por tick.
