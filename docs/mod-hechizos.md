# Mod: Hechizos habilitados, nuevos y libro de hechizos

Commit: `8d2415c2f`

## Resumen

El mod habilita 4 hechizos que originalmente no estaban disponibles en Diablo 1 (3 de Hellfire + 2 de Diablo bloqueados), y agrega 1 hechizo completamente nuevo (DashStrike). También expande el libro de hechizos de 7 a 8 entradas por página para acomodar los nuevos hechizos sin reemplazar los originales.

### Hechizos habilitados/nuevos

| Hechizo | Origen | Cambio | Página libro |
|---|---|---|---|
| Nova | Diablo | Desbloqueado de `GetSpellBookLevel()` | 4 (ya existía) |
| Apocalypse | Diablo | `sBookLvl` 19→7 + desbloqueado | 4 (ya existía) |
| Reflect | Hellfire | Habilitado en modo Diablo | 2 (nuevo slot) |
| Berserk | Hellfire | Habilitado en modo Diablo | 3 (nuevo slot) |
| RingOfFire | Hellfire | Habilitado en modo Diablo | 4 (nuevo slot) |
| DashStrike | **Nuevo** | Hechizo original del mod | 1 (nuevo slot) |

---

## Hechizos de Diablo desbloqueados como libros

### Nova

Nova ya existía en Diablo 1 con `sBookLvl = 14` en `spelldat.cpp`, pero estaba bloqueada por un case en `GetSpellBookLevel()` que retornaba `-1` para ella en modo no-Hellfire.

**Fix:** Eliminado el case `SpellID::Nova` del switch en `GetSpellBookLevel()` (`spells.cpp:325`). Ahora retorna su `sBookLvl` original (14).

### Apocalypse

Apocalypse tenía `sBookLvl = 19` (inalcanzable en un mod de 7 niveles) y estaba bloqueada igual que Nova.

**Fix:**
1. Eliminado el case del switch en `GetSpellBookLevel()` (`spells.cpp:325`)
2. `sBookLvl` cambiado de 19 a 7 en `spelldat.cpp:47` — aparece en tiendas/libros desde nivel 6

---

## Hechizos de Hellfire habilitados en modo Diablo

Estos tres hechizos están completamente implementados en devilutionX (misil, lógica, daño), pero bloqueados por dos funciones gate:

### Gate 1: `IsValidSpell()` (`spells.cpp:70`)

Originalmente rechazaba todo hechizo con `SpellID > LastDiablo` a menos que `gbIsHellfire` fuera true.

```cpp
// ANTES:
return spl > SpellID::Null
    && spl <= SpellID::LAST
    && (spl <= SpellID::LastDiablo || gbIsHellfire);

// DESPUÉS:
return spl > SpellID::Null
    && spl <= SpellID::LAST
    && (spl <= SpellID::LastDiablo
        || spl == SpellID::Reflect
        || spl == SpellID::Berserk
        || spl == SpellID::RingOfFire
        || spl == SpellID::DashStrike
        || gbIsHellfire);
```

### Gate 2: `GetSpellBookLevel()` (`spells.cpp:328`)

Originalmente retornaba `-1` para todo hechizo con `SpellID > LastDiablo` en modo no-Hellfire.

```cpp
// ANTES:
if (!gbIsHellfire) {
    switch (s) {
    case SpellID::Nova:
    case SpellID::Apocalypse:
        return -1;
    default:
        if (s > SpellID::LastDiablo)
            return -1;
        break;
    }
}

// DESPUÉS:
if (!gbIsHellfire) {
    switch (s) {
    case SpellID::Reflect:
    case SpellID::Berserk:
    case SpellID::RingOfFire:
    case SpellID::DashStrike:
        break;
    default:
        if (s > SpellID::LastDiablo)
            return -1;
        break;
    }
}
```

### Datos de los hechizos (de `spelldat.cpp`)

| Hechizo | sBookLvl | Mana | Staff cost | Tipo |
|---|---|---|---|---|
| Reflect | 3 | 300 | 20 | Magic |
| Berserk | 3 | 300 | 20 | Magic + Targeted |
| RingOfFire | 5 | 600 | 40 | Fire |

### Iconos placeholder (`GetSpellIcon()`)

Los sprites de iconos de Hellfire (frames 42, 45, 46) no existen en los archivos de Diablo. Se creó una función wrapper `GetSpellIcon()` en `spell_icons.cpp` que mapea a frames de Diablo existentes como placeholder:

| Hechizo | Frame Hellfire (original) | Frame Diablo (placeholder) | Icono usado |
|---|---|---|---|
| Reflect | 45 | 12 | ManaShield |
| Berserk | 46 | 19 | Rage |
| RingOfFire | 42 | 6 | FireWall |
| DashStrike | 9 | 9 | Mana (ya existe) |

La función se usa en `DrawLargeSpellIcon()` y `DrawSmallSpellIcon()` en vez de acceso directo a `SpellITbl[]`. Cuando se reemplacen con sprites custom, solo hay que cambiar los valores en `GetSpellIcon()`.

### Reflect sprite fallback (`scrollrt.cpp`)

El sprite del misil `Reflect` (archivo `missiles\reflect`) es un asset de Hellfire y no existe en modo Diablo. `DrawPlayerIcons()` usa `MissileGraphicID::ManaShield` como fallback cuando el sprite no está cargado:

```cpp
if (player.wReflections > 0) {
    MissileGraphicID reflectIcon = MissileGraphicID::Reflect;
    if (!GetMissileSpriteData(MissileGraphicID::Reflect).sprites)
        reflectIcon = MissileGraphicID::ManaShield;
    DrawPlayerIconHelper(out, reflectIcon, position + Displacement { 0, 16 }, ...);
}
```

### Notas sobre cada hechizo

**Reflect:** Funciona correctamente. Otorga reflexiones de daño proporcionales al nivel del hechizo × nivel del jugador. El ícono visual sobre el jugador usa ManaShield como fallback. Datos del misil: `misdat.cpp:112` — `AddReflect`, `Physical | Invisible`, sonido `LS_MSHIELD`.

**Berserk:** Funciona correctamente por diseño. No tiene sprite visual (es invisible). Busca el monstruo más cercano en rango 0-5 tiles, le aplica `MFLAG_BERSERK` (ataca a otros monstruos) y aumenta su daño en 120% + random(10%) + slvl. Si no hay monstruos cerca, el hechizo falla silenciosamente (`spellFizzled = true`).

**RingOfFire:** Funciona correctamente. Crea un anillo de fuego alrededor del jugador. Usa el misil `MissileID::RingOfFire` que ya estaba completamente implementado en devilutionX.

---

## Nuevo hechizo: DashStrike

DashStrike es un hechizo original del mod: teletransporta al jugador adyacente a un enemigo y le asesta un golpe melee con daño de arma × 1.5.

### Datos (`spelldat.cpp`)

```
Nombre: "Dash Strike"
Mana: 500, Staff mana: 30, Affix cost: 25
Tipo: Magic | Targeted
sBookLvl: 6, sStaffLvl: 5
Missile: DashStrike + Null
```

### Enums (`spelldat.h`)

- `SpellID::DashStrike` — índice 52, `LAST = DashStrike`
- `MissileID::DashStrike` — después de `OrangeExplosion`
- `MAX_SPELLS` cambiado de 52 a 53

### Datos del misil (`misdat.cpp`)

```cpp
/*DashStrike*/ { &AddDashStrike, &ProcessDashStrike, LS_TELEPORT, SFX_NONE,
                 MissileGraphicID::None, Physical | Invisible,
                 MissileMovementDistribution::Disabled },
```

- Sin sprite visual (`None`)
- Invisible al cliente remoto
- Sonido de Teleport al lanzar

### Implementación (`missiles.cpp`)

#### `AddDashStrike()`

1. Valida que el caster sea un jugador (`TARGET_MONSTERS`)
2. Busca una posición adyacente al target (`FindClosestValidPosition`, rango 0-1) que sea caminable por el jugador (`PosOkPlayer`)
3. Si no encuentra posición → spell fizzled
4. Guarda la posición de destino del misil y la posición del target original en `var1`/`var2`
5. `_mirange = 2` (se ejecuta en el siguiente frame)

#### `ProcessDashStrike()`

1. Decrementa `_mirange`, si llega a 0 → elimina misil
2. Busca posición adyacente a la posición del misil para teletransportar al jugador
3. **Teletransporte:** Actualiza `position.tile`, `position.future`, `position.old`, `dPlayer[]`, luz y visión
4. Calcula dirección hacia el target original
5. **Golpe melee:** Busca monstruo en la tile frente al jugador (`position.tile + dir`)
6. Si hay monstruo: calcula daño de arma completo:
   ```
   mind = _pIMinDam + _pIBonusDamMod
   maxd = _pIMaxDam + _pIBonusDamMod
   dam = random(mind..maxd) + _pIBonusDam% + _pDamageMod
   dam = dam × 1.5
   ```
7. `ApplyMonsterDamage(Physical, monster, dam)` — aplica el daño
8. `PlaySfxLoc(PS_SWING)` — sonido de golpe

### Sincronización MP (`multi.cpp`)

DashStrike se agrega a la lista de hechizos que mueven al jugador en `NetReceivePlayerData()`:

```cpp
if (myPlayer._pmode == PM_SPELL && IsAnyOf(myPlayer.executedSpell.spellId,
    SpellID::Teleport, SpellID::Phasing, SpellID::Warp, SpellID::DashStrike))
    target = {};
```

Esto previene desyncs donde el cliente remoto camina hacia la posición vieja del jugador mientras se ejecuta el teletransporte.

### `GetDamageAmt()` (`missiles.cpp`)

DashStrike retorna `-1, -1` (sin daño fijo) en `GetDamageAmt()`, igual que Berserk y otros hechizos de utilidad. El daño se calcula dinámicamente en `ProcessDashStrike()` basado en el arma equipada.

### Limitaciones conocidas

- **Save/Load:** El índice de DashStrike (52) está fuera del rango de save/load de `pack.cpp` (0-46). El nivel del hechizo no se guarda en saves.
- **No aparece en el panel de MSI**: Las facturas de tienda y staves no generan DashStrike naturalmente — solo via `setspells` debug command o libros custom.

---

## Libro de hechizos: 8 entries por página

### Cambios en `spell_book.cpp`

Para agregar los 4 nuevos hechizos sin reemplazar ningún hechizo original de Diablo, se expandió cada página de 7 a 8 entries, reduciendo la altura de cada entry.

| Parámetro | Antes | Después |
|---|---|---|
| `SpellBookPageEntries` | 7 | 8 |
| `SpellBookDescription.height` | 43px | 37px |
| `lineHeight` (texto) | 18px | 16px |
| `textPaddingTop` | 7px | 5px |
| `iconArea` height | `43 * 7 - 5 = 296` | `37 * 8 - 5 = 291` |

### Verificación de espacio

- Iconos empiezan en y=18, tabs en y=320
- Espacio disponible: 302px
- 8 entries × 37px = 296px → 6px de margen inferior
- Último entry: yp=271, icono bottom=308, gap hasta tabs=12px

### Distribución de hechizos por página

| Página | Slot 8 (nuevo) | Hechizos originales |
|---|---|---|
| 1 (básicos) | **DashStrike** | Skill, Firebolt, ChargedBolt, HolyBolt, Healing, HealOther, Inferno |
| 2 (medios) | **Reflect** | Resurrect, FireWall, Telekinesis, Lightning, TownPortal, Flash, StoneCurse |
| 3 (avanzados) | **Berserk** | Phasing, ManaShield, Elemental, Fireball, FlameWave, ChainLightning, Guardian |
| 4 (poder) | **RingOfFire** | Nova, Golem, Teleport, Apocalypse, BoneSpirit, BloodStar, Etherealize |

Las páginas 5 y 6 (Hellfire) mantienen sus entries originales + `Invalid` en el slot 8.

---

## Archivos modificados

| Archivo | Cambio |
|---|---|
| `Source/spelldat.h` | `MAX_SPELLS` 52→53, `SpellID::DashStrike`, `MissileID::DashStrike`, `LAST = DashStrike` |
| `Source/spelldat.cpp` | Apocalypse `sBookLvl` 19→7, entrada DashStrike en `SpellsData[]` |
| `Source/spells.cpp` | `IsValidSpell()` exceptions, `GetSpellBookLevel()` exceptions |
| `Source/misdat.cpp` | Entrada DashStrike en `MissilesData[]` |
| `Source/missiles.cpp` | `AddDashStrike()`, `ProcessDashStrike()`, case en `GetDamageAmt()` |
| `Source/missiles.h` | Declaraciones `AddDashStrike()`, `ProcessDashStrike()` |
| `Source/multi.cpp` | `DashStrike` en lista de hechizos que mueven jugador para sync MP |
| `Source/panels/spell_book.cpp` | 8 entries, height 37px, SpellPages con nuevos hechizos, iconArea ×8 |
| `Source/panels/spell_icons.cpp` | `GetSpellIcon()` wrapper, `SpellITbl` entry para DashStrike (frame 9) |
| `Source/panels/spell_icons.hpp` | Declaración `GetSpellIcon()` |
| `Source/engine/render/scrollrt.cpp` | Reflect sprite fallback a ManaShield en `DrawPlayerIcons()` |
