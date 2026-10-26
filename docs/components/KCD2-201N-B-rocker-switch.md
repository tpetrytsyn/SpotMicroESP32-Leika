# KCD2-201N-B rocker switch

The robot's main power switch — the only on/off control, and the only state in which it draws
nothing. **Decided, not bought.** Sourced as
["Перемикач Rocker KCD2-201N-B Daier"](https://www.rcscomponents.kiev.ua/product/peremykach-rocker-kcd2-201n-b_211755.html)
(RCS Components), 35 ₴.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Configuration | DPST, ON-OFF, latching | listing |
| Rating | 30 A / 250 V AC — **no DC rating** | listing |
| Lamp | red, likely neon | listing |
| Terminals | tabs with holes | listing |
| Panel cutout | **22 × 30.8 mm** | listing |
| Body | 32 × 25 × 36 mm | listing |

30 A is an AC figure and optimistic for this size (15–20 A is typical) — treat it as unverified.
**Alternative on record:** the DC-rated [ASW-07D-2 toggle](alternatives/ASW-07D-2-toggle-switch.md),
which fits Kubina's 19 mm rear-cover hole with a reducer. Swap to it if this cutout doesn't fit the
shell, or at 3S.

**Why AC-only is acceptable here:** DC has no zero crossing to quench an arc, but 8.4 V is below
the ~12 V needed to sustain one, so breaking ~12 A is gentle on the contacts **[inferred]**. 2S
only. The switch-on surge into the converters' capacitors lasts milliseconds.

## Use in this project

In pack `+` between the F114-C and the + split ([wiring.md](../wiring.md#power-path)). Off means
zero draw.

- **One pole.** Don't parallel the second — contact resistance never matches, so one pole carries
  most of the current anyway. Leave it unconnected.
- **Switch `+`, never `−`**; ground stays continuous to the star.
- **Solder 14 AWG** through the tab holes and heat-shrink each joint — push-on spades walk off under
  vibration. Strain-relieve both wires.
- **Lamp:** a neon needs ~70 V to strike **[inferred]**, so it stays dark at 8.4 V. Leave its
  terminal unconnected until the delivered part is checked.

## Operating it

- **Deactivate the servos first** — the rocker cuts servo power instantly, and a standing robot
  drops.
- **Unplug the XT60 too** for storage and transport: pressed on in a bag, the rocker drains the pack.
- **It's the only way out of UI sleep** — no wake button is fitted
  ([Not fitted](../wiring.md#not-fitted)). Prefer switching off.

## Open items

- [ ] The lamp type on the delivered part
- [ ] The 22 × 30.8 mm cutout, reachable without lifting the robot — Kubina's rear cover has only a
      round 19 mm hole, so this needs a reworked cover, or the toggle
