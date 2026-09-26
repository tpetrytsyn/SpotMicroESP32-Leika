# CN3903 fixed 5 V / 3 A buck module

Step-down converter for the ESP32's 5 V rail. Sourced as
["Понижуючий перетворювач CN3903 5V 3A"](https://myproject.com.ua/ponyzhuiuchyi-peretvoriuvach-cn3903-5v-3a.html)
(myproject.com.ua).

**Fixed output, no adjustment.** That is the reason it is the documented choice here over
the adjustable [LM2596S CC/CV module](alternatives/LM2596S-CC-CV-module.md) — see
[Why fixed output](#why-fixed-output-matters-here).

Confidence follows [ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md](ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md):
**[photo]** = read off the product images, **[listing]** = stated by the seller,
**[inferred]** = reasoned, needs confirmation on hardware.

## Specifications

| Item | Value | Source |
| --- | --- | --- |
| Output voltage | **5 V, fixed** | **[listing]** |
| Input voltage | 5.5 – 30 V; seller recommends ≤ 28 V | **[listing]** |
| Max output current | 3 A peak, **1.6 – 1.8 A continuous** | **[listing]** |
| Efficiency | up to 96% | **[listing]** |
| Topology | Buck, synchronous | **[listing]** + **[inferred]** |
| Operating temperature | −40 … +85 °C | **[listing]** |
| Dimensions | 17.5 × 12 × 4.5 mm | **[listing]** |
| Connections | Solder pads: `IN+`, `IN−`, `OUT+`, `OUT−` | **[listing]** + **[photo]** |

Construction **[photo]**: SO-8 regulator, 4.7 µH shielded inductor (`4R7`), ceramic
input and output capacitors with no electrolytics, four corner mounting pads.

The small inductor and all-ceramic filtering indicate a **high switching frequency**,
likely several hundred kHz or more — consistent with the efficiency claim and with low
output ripple. **[inferred]**

> **The part number is the seller's, not the silicon's.** The chip carries the markings
> `GKE0E` / `GMC2X` **[photo]**, which are lot codes rather than a part number. Nothing in
> the photos confirms the regulator is actually a CN3903, so the figures above are vendor
> claims. This does not affect the recommendation — the module's measured behaviour is
> what matters — but do not go looking for a datasheet to design against.

## Why fixed output matters here

The alternative on hand, the [LM2596S CC/CV module](alternatives/LM2596S-CC-CV-module.md), has three
multi-turn trimmers, none labelled by function. Two consequences this module avoids
entirely:

- **Nothing to knock out of adjustment.** On a walking robot that vibrates and gets
  handled, a trimmer is a liability — a quarter turn can put 8 V into the ESP32. There is
  no such failure mode on a fixed module.
- **No CC pot to brown out the load.** The LM2596S variant is a charger: in
  constant-current mode it drops output voltage to hold the limit, so a low CC setting
  sags the rail during Wi-Fi bursts.

Setup reduces to soldering four wires.

## Use in this project

Feeding the ESP32's 5 V from the shared 2S battery rail
([wiring.md](../wiring.md)):

| | |
| --- | --- |
| Input | 6.0 – 8.4 V (2S across its discharge) — inside 5.5–30 V throughout |
| Output | 5.0 V fixed |
| Load | ESP32-S3 with camera, a few hundred mA |
| Headroom | ~¼ of the *conservative* 1.6 A continuous rating |
| Thermal | At ~2.5 W out and ~96% efficient, roughly 0.1 W dissipated — runs cold |
| Across discharge | Holds 5 V down to ~5.5 V in, so it outlasts the servo rail |

### Wiring

| Pad | To |
| --- | --- |
| `IN+` | battery positive |
| `IN−` | common ground |
| `OUT+` | ESP32 `VCC` |
| `OUT−` | common ground |

Solder pads rather than terminals. Inconvenient on the bench, but on a moving robot a
soldered joint is more reliable than a screw terminal — give the wires strain relief so
the pads do not take mechanical load.

## Before connecting the ESP32

**Meter the output.** Power it from the pack voltage with nothing on the output and
confirm **5.0 V**.

Fixed-output modules at this price are occasionally built with the wrong feedback divider,
and unlike an adjustable module there is no way to correct it — the first thing that finds
out would be the ESP32.

## Open items

- [ ] **Confirm 5.0 V on the bench** before first use.
- [ ] **Actual regulator part**, unresolved — the marking is a lot code.
- [ ] **Output ripple** under the ESP32's Wi-Fi transmit bursts, if brownouts ever appear.
- [ ] **Real continuous rating** with the intended mounting. The seller's 1.6–1.8 A
      derating is honest for a module this size, and the load here is far below it.
