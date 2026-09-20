# Alternatives — evaluated, not used

Parts that were considered for this build and **deliberately not chosen**. Kept because
the findings cost real effort to establish and the hardware is usually still on the shelf,
so the decision may get revisited.

Nothing here describes the robot as built. For that, see the parent
[components/](../) folder.

Each document states at the top what superseded it and why.

| Part | Role considered for | Superseded by | Reason |
| --- | --- | --- | --- |
| [LM2596S CC/CV module](LM2596S-CC-CV-module.md) | 5 V rail for the ESP32 | [CN3903](../CN3903-5V-buck-module.md) | Three unlabelled trimmers, and a CC pot that browns out the load if set low. The CN3903 is fixed-output with nothing to misadjust. |

## When to add something here

Move a component document into this folder when a part is ruled out **after** it has been
written up — not when a part is merely never bought. The value is the recorded reasoning:
what was wrong with it, and what replaced it. A part nobody evaluated needs no document at
all.
