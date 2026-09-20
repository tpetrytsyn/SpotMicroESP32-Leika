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

Parts **evaluated and not chosen**, kept for reference in
[components/alternatives/](components/alternatives/):

- [LM2596S CC/CV module](components/alternatives/LM2596S-CC-CV-module.md) — adjustable 5 V, superseded by the CN3903

Procedures that use them:

- [Servo calibration](servo-calibration.md) — bench components, wiring and parameters

## About Spot

<!-- - [Kinematics](kinematics.md) (transformation matrix, mode etc)-->
- [API](api.md)
- [Robots capabilities](spot.md)
