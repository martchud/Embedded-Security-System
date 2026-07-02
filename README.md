# Embedded Security System - A networked home-security alarm built on the BeagleBone Green

The system combines PIR motion sensing, a live webcam stream, QR-code disarming, SMS intruder alerts, and two web dashboards — all driven by a multithreaded C application talking to Node.js servers over UDP.

## Overview

When armed, the system monitors a room with a PIR motion sensor. Sustained motion triggers a 20-second countdown (shown on a 14-segment display with an accelerating warning LED and audible beeps). If the countdown expires, the alarm sounds through a speaker while an 8×8 LED matrix flashes red. The system can be disarmed four ways:

- Typing the 4-digit passcode on a USB numeric keypad connected to the BeagleBone
- Entering the passcode on web dashboard
- Holding a valid QR code up to the webcam, the live stream is scanned server-side about once per second, and a match disarms the alarm automatically
- Sending a `disarm` command directly to the device's UDP interface

A one-time QR passcode can be regenerated from the browser at any time and delivered to a phone via Twilio MMS. If the alarm reaches the sounding state, the server texts an intruder alert via Twilio SMS.

## System Architecture

The project is split across three cooperating programs connected by UDP over an Ethernet-over-USB link (BeagleBone `192.168.7.2` ↔ host PC `192.168.7.1`):

```
┌────────────────────── BeagleBone Green ──────────────────────┐
│  security_system (C, pthreads)                               │
│   ├─ Alarm state machine (unarmed → armed → alerted →        │
│   │                        sounding)                         │
│   ├─ PIR motion sensor thread (GPIO)                         │
│   ├─ USB keypad passcode entry (Linux input events)          │
│   ├─ 14-seg countdown display + 8×8 LED matrix (I2C)         │
│   ├─ ALSA audio (warning beeps / alarm siren), pot-controlled│
│   │   volume, Zen cape joystick                              │
│   ├─ UDP command server        ◄──── port 12345 ────┐        │
│   └─ V4L2 webcam capture (H.264) ── UDP :1234 ──►   │        │
│                                                     │        │
│  server-code (Node.js, port 3042)                   │        │
│   └─ Control dashboard: HTTP + Socket.IO → UDP ─────┘        │
└──────────────────────────────────────────────────────────────┘
                                │ frames        ▲ commands
                                ▼               │
┌────────────────────────── Host PC ───────────────────────────┐
│  streamserver (Node.js + Express + Socket.IO, port 3000)     │
│   ├─ ffmpeg: UDP H.264 → MJPEG → live <canvas> stream        │
│   ├─ jsQR + Jimp: scans a frame ~1×/sec, disarms on match    │
│   ├─ qrcode: generates new random QR passcodes               │
│   └─ Twilio: MMS QR code to phone, SMS intruder alerts       │
└──────────────────────────────────────────────────────────────┘
```

**C application (`main.c` + modules).** Each hardware subsystem runs on its own POSIX thread and coordinates through a shared `Alarm` struct. The core logic thread evaluates the state machine once per second; the motion thread integrates PIR readings (incrementing on motion, decaying otherwise) so only sustained motion escalates the state. The webcam thread is streaming 720×720 H.264 frames over UDP and optionally running a lightweight motion detector that computes the sum of absolute differences on a sampled band of each frame (whole-frame diffing proved too slow on the ARM target).

**Stream server (`streamserver/`).** Runs on the host PC. Pipes the incoming H.264 stream through `ffmpeg` into MJPEG and pushes frames to the browser over Socket.IO for a live view. Roughly once per second it snapshots a frame (Jimp JPEG→PNG conversion, throttled to avoid malformed-image races), scans it with jsQR, and — on a passcode match — fires a `QR` disarm command back to the BeagleBone and shows a "deactivated" banner. It also generates fresh QR passcodes with error-correction level H, and integrates the Twilio API for MMS/SMS delivery.

**Control dashboard (`server-code/`).** A lightweight Node.js server on the BeagleBone that serves a status page and bridges browser Socket.IO events to the local UDP command interface. It polls `getStatus` every second and renders live state (Disarmed / Armed / INTRUDER DETECTED), supports arm/disarm with passcode validation (invalid attempts pop a modal), passcode changes, and remote shutdown, with a timeout warning if the C application isn't responding.

## Alarm State Machine

| State | Meaning | Behaviour |
|---|---|---|
| `0` unarmed | System off | Motion ignored; counters reset |
| `1` armed | Watching | PIR events accumulate; sustained motion → alerted |
| `2` alerted | Countdown | 20 s timer on 14-seg display, warning beeps, status LED blinks faster (1→2→4 Hz) as time runs out |
| `3` sounding | Intruder | Alarm siren loops, 8×8 matrix flashes stop symbol, SMS alert sent |

Passcodes are 4-digit integers persisted to `password.txt`, loaded at startup and updated atomically on `reset` commands so changes survive reboots.

## UDP Command Protocol (port 12345)

Testable with `netcat -u 192.168.7.2 12345`:

| Command | Action |
| `getStatus` | Returns current state (0–3) |
| `arm <code>` | Arms the system (passcode-validated) |
| `disarm <code>` | Disarms the system (passcode-validated) |
| `reset <old> <new>` | Changes the passcode; persists to file |
| `QR` | Disarm issued by the stream server on a QR match |
| `stop` | Graceful shutdown of all threads |
| `help` | Lists commands |

## Hardware

- BeagleBone Green (Debian 11.5) with Zen cape (joystick, potentiometer, status LED, 14-segment I2C display)
- PIR motion sensor (GPIO)
- External 8×8 LED matrix (HT16K33, I2C)
- USB webcam (720p/1080p) and USB numeric keypad
- Audio output via ALSA for warning/alarm WAV playback; volume controlled by the potentiometer

## Repository Layout

```
├── main.c                  # Entry point; spins up all subsystems
├── alarm_interface.c/.h    # Alarm struct, state machine, passcode persistence
├── motion.c                # PIR sensor thread (GPIO sysfs)
├── camera.c                # V4L2 H.264 capture → UDP + frame-diff motion detect
├── keyboard.c              # USB numpad passcode entry (/dev/input events)
├── udp.c                   # UDP command server (port 12345)
├── 14segCountdown.c        # I2C 14-seg countdown display
├── ext_8x8led.c            # HT16K33 8×8 matrix driver (stop symbol)
├── led_status.c            # Status LED with variable blink rate
├── stopwatch.c             # Countdown timer thread
├── audio*.c                # ALSA mixer, buffers, alarm/beep playback
├── volKnob.c, joystick.c   # Zen cape volume pot + joystick arm/disarm
├── terminal.c              # Live console status output
├── include/                # Module headers
├── wave-files/             # Alarm and beep WAV assets
├── server-code/            # BeagleBone web dashboard (Node.js, port 3042)
├── streamserver/           # Host-PC stream/QR/Twilio server (Node.js, port 3000)
└── makefile                # Cross-compile + deploy to NFS share
```

## Build & Run

**Prerequisites:** `arm-linux-gnueabihf-` cross-compiler toolchain, Node.js + npm on both machines, `ffmpeg` on the host PC, an NFS share mounted on the BeagleBone at `/mnt/remote/myApps`, and Ethernet-over-USB networking between host and target.

```bash
# On the host: cross-compile the C app and stage everything
# (binary, WAV assets, password.txt, dashboard server) to the NFS share
make

# On the BeagleBone: start the security system
cd /mnt/remote/myApps
./security_system            # add "motion" to enable camera-based motion detection
node security-server-copy/server.js    # control dashboard on :3042

# On the host PC: start the stream/QR server
cd streamserver && npm install && node index.js   # live view on :3000
```

The app compiles with `-Wall -Werror -Wshadow` under C99 and links against pthreads and ALSA.

## Troubleshooting

- **"couldn't find enough finder patterns"** — the qrcode-reader library frequently fails on partial detections; jsQR is used for scanning instead.
- **"Malformed data passed to binarizer"** — caused by frames being overwritten mid-scan; QR scanning is throttled to once per second (increase the interval if it recurs).
- **Choppy stream** — reduce the capture frame size and refresh the browser.
- **Webcam not detected** — verify with `usb-devices` on the BeagleBone and check the camera's max power draw if using a USB splitter.

## Tech Stack

**C (POSIX threads, V4L2, ALSA, BSD sockets, Linux sysfs GPIO/I2C)** · **Node.js (Express, Socket.IO)** · **ffmpeg** · **jsQR / qrcode / Jimp** · **Twilio API** · **ARM cross-compilation (GNU Make)**

## Authors

Jason Chung, Martin Chudy, Josh Murphy, Caleb Bradley

## Acknowledgments

Webcam capture adapted from Derek Molloy's [boneCV](https://github.com/derekmolloy/boneCV), BeagleBone-to-Node streaming approach based on SFU CMPT 433 course guides, QR scanning via [jsQR (https://github.com/cozmo/jsQR).
