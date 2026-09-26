# KCD2-201N-B rocker switch

The robot's main power switch — the only on/off control, and the only state in which the
robot draws nothing. **Decided, not yet bought.** Sourced as
["Перемикач Rocker KCD2-201N-B Daier"](https://www.rcscomponents.kiev.ua/product/peremykach-rocker-kcd2-201n-b_211755.html)
(RCS Components), 35 ₴.

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[listing]** = stated by the seller, **[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Configuration | DPST — two poles, ON-OFF, latching rocker | **[listing]** |
| Rating | 30 A / 250 V AC | **[listing]** — **no DC rating given** |
| Illumination | red | **[listing]** — likely a neon lamp, see below |
| Terminals | tabs with holes | **[listing]** |
| Panel cutout | **22 × 30.8 mm** | **[listing]** |
| Body | 32 × 25 × 36 mm | **[listing]** |
| Manufacturer | Daier | **[listing]** |

> **30 A is an AC figure, and optimistic for a switch this size.** Rockers in this body are
> more commonly rated 15–20 A. Treat the rating as unverified.

> **Alternative on record:** the [ASW-07D-2 toggle](alternatives/ASW-07D-2-toggle-switch.md)
> is DC-rated and takes a round 12.2 mm hole, which suits the Kubina rear cover's 19 mm
> button hole with a reducer ring. Swap to it if this rocker's 22 × 30.8 mm cutout does not
> fit the shell.

## Why an AC-only rating is acceptable here

Breaking DC is harder than AC because DC has no zero crossing to extinguish the arc. At
**8.4 V**, though, the pack is below the ~12 V needed to sustain an arc across opening
contacts, so interrupting ~12 A is gentle on them **[inferred]**. This holds for 2S only —
at 3S or above, choose a switch with a stated DC rating.

The switch-on surge into the converters' input capacitors lasts milliseconds and is well
within a 30 A-class contact.

## Use in this project

**In the pack `+` line, between the F114-C fuse and the ACS712** — see
[wiring.md](../wiring.md#power-path). Everything downstream, including the sensors, is off
when it is off, so the robot draws **zero** from the pack.

- **Use one pole only.** Do not parallel the two poles to share current — contact
  resistance never matches, so one pole carries most of it anyway. Leave the second pole
  unconnected.
- **Switch `+`, never `−`.** The ground stays continuous to the star point.
- **Solder 14 AWG to the tabs** through the holes, and cover each joint with heat-shrink.
  Push-on spade connectors can walk off under vibration.
- **Strain-relieve both wires** so the tabs carry no load when the body flexes.

### The lamp

"N" versions of these rockers typically carry a **neon** lamp for mains, which needs
~70 V or more to strike **[inferred]**. At 8.4 V it will stay dark — harmless, but the
switch is effectively unlit. Leave the lamp terminal unconnected rather than guessing its
wiring; the robot's own LEDs and UI already show that it is running.

## Operating it

**Deactivate the servos in the UI before switching off.** The rocker cuts servo power
instantly, and a robot standing under load will drop. Lie it down or deactivate first,
then switch off.

**It is also the only way to wake the robot from UI sleep.** No wake button is fitted —
see [Not fitted](../wiring.md#not-fitted) — so after `/api/system/sleep` the robot stays
asleep until the rocker power-cycles it. Prefer switching off over UI sleep.

## Open items

- [ ] **Confirm the lamp type** on the delivered part, before connecting its terminal.
- [ ] **Plan the 22 × 30.8 mm cutout** in the body, reachable without lifting the robot.
      Kubina's rear cover has only a round 19 mm button hole, so this needs a reworked
      cover — or the [toggle alternative](alternatives/ASW-07D-2-toggle-switch.md).
