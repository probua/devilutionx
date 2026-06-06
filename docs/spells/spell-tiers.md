# Spell Tiers — Sistema de niveles de libros

## Objetivo

Organizar los hechizos en 4 tiers que se corresponden con las 4 páginas del libro de hechizos. Cada tier tiene un `sBookLvl` distinto que controla desde qué nivel de la mazmorra pueden dropear sus libros.

## Cómo funciona el dropeo de libros

El proceso tiene dos filtros:

1. **Filtro de item base** (`iMinMLvl`): controla qué item base (BOOK1/BOOK2/BOOK3/BOOK4) puede aparecer en un nivel de mazmorra. Se compara con `ItemsGetCurrlevel() * 2` (que usa `GetVirtualLevel()`). En drops de monstruo (`RndUItem`) los libros siempre pasan este filtro.

2. **Filtro de hechizo** (`sBookLvl`): `GetBookSpell()` itera todos los `SpellID` del 1 al 37 (Diablo), y para cada uno verifica `sBookLvl != -1 && lvl >= sBookLvl`. Entre los elegibles, selecciona uno al azar con igual probabilidad.

## Mapeo página → tier → sBookLvl

| Página | Tier | sBookLvl | Hechizos |
|---|---|---|---|
| 0 | 1 | 1 | Firebolt, ChargedBolt, HolyBolt, Healing, Telekinesis, Raise Skeleton |
| 1 | 2 | 3 | FireWall, Inferno, Lightning, TownPortal, Flash, StoneCurse |
| 2 | 3 | 5 | Phasing, ManaShield, Elemental, Fireball, FlameWave, ChainLightning, Guardian |
| 3 | 4 | 6 | Nova, Golem, Teleport, Apocalypse, BoneSpirit, BloodStar |

Los hechizos con `sBookLvl = -1` nunca aparecen como libro: Identify, Infravision, Resurrect, DoomSerpents, BloodRitual, Invisibility, Etherealize, Rage, y todos los de Hellfire (Mana, Magi, Jester, runas, etc.).

## Progresión por nivel de mazmorra

`GetBookSpell()` recibe `lvl = ItemsGetCurrlevel() = GetVirtualLevel()`. La elegibilidad se determina con `lvl >= sBookLvl`:

| currlevel | Virtual level | sBookLvl elegibles |
|---|---|---|
| 1 | 1 | Tier 1 |
| 2 | 3 | Tier 1–2 |
| 3 | 5 | Tier 1–3 |
| 4 | 7 | Tier 1–4 |
| 5 | 9 | Todos |
| 6 | 13 | Todos |
| 7 | 16 | Todos |

El sistema es **acumulativo**: los tiers inferiores siempre están disponibles, y al avanzar de nivel se agregan los tiers superiores al pool de selección aleatoria.

## Item bases de libros

| Item ID | iMinMLvl (original) | iMinMLvl (mod) |
|---|---|---|
| IDI_BOOK1 | 2 | 2 |
| IDI_BOOK2 | 8 | 6 |
| IDI_BOOK3 | 14 | 10 |
| IDI_BOOK4 | 20 | 12 |

Ajustados para que BOOK2 esté disponible desde nivel 2 (Virtual=3, 3×2=6), BOOK3 desde nivel 3 (Virtual=5, 5×2=10), y BOOK4 desde nivel 4 (Virtual=7, 7×2=14).

**Nota:** `maxSpells` se fija en `static_cast<int>(SpellID::LastDiablo) + 1` (= 38) en vez del vanilla `37`, para incluir `Skeleton` (SpellID=37) que antes quedaba excluido por el `if (s == maxSpells) s = 1`. Misma corrección en `GetStaffSpell()`.

## Algoritmo GetBookSpell()

```
lvl = ItemsGetCurrlevel()  → GetVirtualLevel()
maxSpells = 37 (Diablo) o MAX_SPELLS (Hellfire)
rv = GenerateRnd(maxSpells) + 1  → número del 1 al maxSpells
s = SpellID::Firebolt (id 1)
mientras rv > 0:
    sLevel = GetSpellBookLevel(s)  → sBookLvl
    si sLevel != -1 && lvl >= sLevel:
        rv -= 1
        bs = s  → hechizo candidato
    s += 1
    si !gbIsMultiplayer && s == Resurrect: s = Telekinesis  → salta Resurrect en SP
    si s == HealOther: s = BloodStar  → salta HealOther siempre
    si s == maxSpells: s = 1  → wrap del loop
```

El hechizo seleccionado determina el color del libro vía `spellData.type()`: `Fire` → rojo, `Lightning` → azul, `Magic` → gris.

## Color de libro por tier

| Tier | Hechizos | Color libro |
|---|---|---|
| 1 | Firebolt | Rojo |
| 1 | ChargedBolt | Azul |
| 1 | HolyBolt, Healing, Telekinesis, Raise Skeleton | Gris |
| 2 | FireWall, Inferno | Rojo |
| 2 | Lightning, Flash | Azul |
| 2 | TownPortal, StoneCurse | Gris |
| 3 | Elemental, Fireball, FlameWave, Guardian | Rojo |
| 3 | ChainLightning | Azul |
| 3 | Phasing, ManaShield | Gris |
| 4 | Apocalypse | Rojo |
| 4 | Nova, Golem, Teleport, BoneSpirit, BloodStar | Gris |

## Archivos modificados

- `Source/spelldat.cpp` — `sBookLvl` de cada hechizo ajustado según página del libro
- `Source/itemdat.cpp` — `iMinMLvl` de BOOK2/BOOK3/BOOK4 reducidos para alinear con tiers
- `Source/items.cpp` — `maxSpells` corregido de 37 a `LastDiablo + 1` en GetBookSpell y GetStaffSpell
