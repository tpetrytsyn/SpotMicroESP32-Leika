# LM2596S CC/CV buck module (3-pot charger variant)

> **Not used.** The 5 V rail is the fixed [CN3903](../CN3903-5V-buck-module.md) — nothing to
> misadjust on a moving robot. Kept as the adjustable alternative, useful wherever a non-5 V rail
> is needed.

Sourced as
["Понижуючий перетворювач LM2596S"](https://prom.ua/ua/m1440220616950486746-ponizhayuschij-preobrazovatel-lm2596s.html?p=2826593351)
(prom.ua); regulator LM2596S-ADJ. This is the **CC/CV charger variant** — three trimmers, three
LEDs — not the common single-pot board.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Input | 5–35 V, must exceed the output | listing |
| Output | 1.25–30 V, adjustable | listing |
| Current | 3 A; heatsink above 15 W | listing |
| Efficiency | up to 92 % | listing |
| Regulation | constant current and constant voltage | listing + photo |
| Protection | overcurrent | listing |
| Temperature | −40…+85 °C | listing |
| Size | 48 × 23 mm | listing |

Parts: 330 µH shielded inductor, 2 × 220 µF 35 V, SK54 Schottky. `IN+`/`IN−` screw terminal at
one end, the output terminal at the other — its polarity isn't legible in the photos, so meter it.
LEDs: `OK`, `CH`, `CC/CV`.

## Identify the trimmers

Marked only `103`, `104`, `W103` (10 k, 100 k, 10 k); listings disagree on which is which. All are
multi-turn.

1. Input only, meter on the output: the trimmer that moves the unloaded voltage is **CV**.
2. With a modest load: the one that makes the output droop as you turn it down is **CC**.
3. The last sets the LED threshold only.

## The CC pot browns out the load

In CC mode the module is a current source: past the limit, the output voltage drops. With CC near
minimum, a 5 V rail sags on Wi-Fi bursts and camera init. For an ESP32-S3 with camera, set CC to
~1 A; the `CC/CV` LED should sit in CV.

**As the 5 V rail:** 6.0–8.4 V in leaves only 1–3.4 V of headroom; ~2.5 W against a 15 W rating
needs no heatsink.

## Open items

- [ ] Which trimmer is which — label the board
- [ ] Output terminal polarity
- [ ] Whether `OK`/`CH` mean anything outside charging
