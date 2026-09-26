# Welcome to the docs

Here you will find the current documentation for the ESP32 based spot micro project, Leika.

## Guide

These are the steps it takes to get a fresh new robot up and barking.

1. [Components](1_components.md)
1. [Assembly](2_assembly.md)
1. [Software](3_software.md)
1. [Turning on for the first time](4_configuring.md)
1. [Running](5_running.md)
1. [Developing](6_developing.md)
1. [Contributing](7_contributing.md)

## Hardware components

Parts **used in this build** — pinouts, specifications, and the quirks found bringing
each one up.

- [ESP32-S3-CAM N16R8 (HW-679)](components/ESP32-S3-CAM-N16R8-Wi-Fi-Bluetooth.md) — controller board
- [CH343P USB-Serial (WeAct)](components/CH343P-USB-Serial-WeAct.md) — flashing adapter
- [DS3235 / TD-8135MG servo](components/DS3235-TD-8135MG-servo.md) — 35 kg leg servo
- [SZBK07 buck converter](components/SZBK07-buck-converter.md) — 300 W CC/CV, servo rail
- [CN3903 buck module](components/CN3903-5V-buck-module.md) — fixed 5 V for the controller
- [ZOP Power 2S 1500 mAh LiPo](components/ZOP-Power-2S-1500mAh-LiPo.md) — main battery, no BMS
- [Daier F114-C / F117-C fuse holders](components/Daier-blade-fuse-holders.md) — 15 A main and 2 A branch fuses
- [KCD2-201N-B rocker switch](components/KCD2-201N-B-rocker-switch.md) — main power switch, the only on/off
- [ADS1115 ADC](components/ADS1115-16bit-ADC.md) — 16-bit, 4 analog inputs over I2C, for monitoring
- [ACS712 30 A current sensor](components/ACS712-30A-current-sensor.md) — pack current
- [Voltage sensor 0–25 V](components/Voltage-sensor-0-25V-divider.md) — ÷5 divider, pack voltage

Parts **evaluated and not chosen**, kept for reference in
[components/alternatives/](components/alternatives/):

- [LM2596S CC/CV module](components/alternatives/LM2596S-CC-CV-module.md) — adjustable 5 V, superseded by the CN3903
- [ASW-07D-2 toggle switch](components/alternatives/ASW-07D-2-toggle-switch.md) — DC-rated main switch, the alternative to the rocker

Procedures that use them:

- [Wiring and power](wiring.md) — the harness: power path, fuses, grounding, I2C, monitoring
- [Servo calibration](servo-calibration.md) — bench setup and per-servo parameters

## About Spot

<!-- - [Kinematics](kinematics.md) (transformation matrix, mode etc)-->
- [API](api.md)
- [Robots capabilities](spot.md)
