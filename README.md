# Kernel-PUBG-VN (Android Vulkan Native Overlay)

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![Platform](https://img.shields.io/badge/Platform-Android%20arm64--v8a-green.svg)](https://developer.android.com/ndk)
[![Render API](https://img.shields.io/badge/Render-Vulkan-red.svg)](https://www.vulkan.org/)
[![GUI](https://img.shields.io/badge/GUI-Dear%20ImGui-orange.svg)](https://github.com/ocornut/imgui)
[![License](https://img.shields.io/badge/License-Educational%20Only-lightgrey.svg)](#disclaimer)

A research project investigating low-level graphics rendering via **Vulkan Native Surface Overlay** and Linux Input Subsystem (`/dev/input`) touch event simulation on 64-bit Android systems (`arm64-v8a`).

---

## 📑 Table of Contents
- [1. System Architecture](#1-system-architecture)
- [2. Directory Structure](#2-directory-structure)
- [3. Environment & Prerequisites](#3-environment--prerequisites)
- [4. NDK Build Guide](#4-ndk-build-guide)
- [5. Installation & Execution](#5-installation--execution)
- [6. Project Documentation](#6-project-documentation)
- [7. Anti-Cheat & Integrity Mechanisms Overview](#7-anti-cheat--integrity-mechanisms-overview)
- [8. Disclaimer](#8-disclaimer)

---

## 1. System Architecture

```
+-------------------------------------------------------------+
|                      User-space Overlay                     |
|  +-------------------------------------------------------+  |
|  |           Dear ImGui GUI + Render Pipeline            |  |
|  +-------------------------------------------------------+  |
|  |             Vulkan Native Surface Creator             |  |
|  +-------------------------------------------------------+  |
|  |     Touch Helper Simulation (/dev/input/event*)       |  |
|  +-------------------------------------------------------+  |
|  |             c_driver (IPC Kernel Client)              |  |
+------------------------------+------------------------------+
                               | Netlink 17 / Syscall Hook
+------------------------------v------------------------------+
|                        Kernel-space                         |
|   Linux Kernel Module (.ko) / KernelSU / APatch Driver      |
+-------------------------------------------------------------+
```

1. **Graphics Rendering**:
   - Implements `ANativeWindowCreator` to allocate a hardware overlay surface directly from Android's `SurfaceFlinger` using Vulkan API, eliminating the need for standard `SYSTEM_ALERT_WINDOW` permissions.
   - Includes anti-screen-recording flags via OS surface properties.
2. **Graphical User Interface (GUI)**:
   - Integrates **Dear ImGui (v1.8x)** for configuration menus, radar views, and real-time world-space transformation displays.
3. **Touch Input Emulation**:
   - `TouchHelper` accesses the Linux input event subsystem (`/dev/input/event*`) directly to simulate multi-touch events without requiring Android Accessibility Services.
4. **Kernel Driver IPC (Memory Interface)**:
   - Supports dual IPC channels for user-space to kernel-space memory exchange:
     - **Netlink Socket**: Raw internal network socket `socket(PF_NETLINK, SOCK_RAW, 17)`.
     - **Syscall Hooking**: Intercepts unused system call routines (such as `lookup_dcookie`) for fast cross-space data transfer.

---

## 2. Directory Structure

```
jni/
├── Android.mk                  # NDK build script declaring source files & compiler flags
├── Application.mk              # Toolchain declarations (arm64-v8a, C++17, c++_static)
├── BAO_CAO_UPDATE_OFFSET.md    # Detailed UE4 structure analysis & offset update guide
├── HUONG_DAN_CAI_DAT.md        # Technical guide for building, kernel module loading & troubleshooting
├── README.md                   # Project overview & documentation
├── include/                    # Header files and dependencies
│   ├── Android_draw/           # Graphics drawing pipeline declarations
│   ├── Android_touch/          # Touch input event handlers
│   ├── Android_vulkan/         # Vulkan graphics wrapper & swapchain headers
│   ├── ImGui/                  # Dear ImGui library & custom font loaders
│   ├── native_surface/         # ANativeWindow interface headers
│   └── Tools/                  # Helper classes (c_driver, vector/matrix math, structs)
└── src/                        # C++ source code implementation
    ├── main.cpp                # Application entry point & lifecycle management
    ├── UI.h                    # ImGui menu layout & user controls
    ├── Aim/                    # Aim calculation & coordinate compensation
    ├── Android_draw/           # UE4 entity iteration & Matrix WorldToScreen projection
    ├── Android_touch/          # Linux event-based touch event implementation
    ├── Android_vulkan/         # Vulkan Instance, device creation & framebuffers
    └── ImGui/                  # ImGui platform backends (Vulkan + Android)
```

---

## 3. Environment & Prerequisites

* **Build Toolchain**:
  - Android NDK (Recommended: **r21e** to **r25c**).
  - Operating System: Windows (PowerShell/CMD), macOS, or Linux.
* **Target Hardware**:
  - Rooted Android device running **KernelSU**, **APatch**, or **Magisk**.
  - CPU Architecture: **ARM64 (aarch64 / arm64-v8a)**.
  - Compatible Linux Kernel Module (`.ko`) installed matching the device's kernel version.

---

## 4. NDK Build Guide

### Step 1: Open Terminal in the project directory
Navigate to the directory containing the `jni/` folder (or enter the `jni/` directory directly):

```bash
cd /path/to/project
```

### Step 2: Run `ndk-build`

```bash
# Build 64-bit arm64-v8a using all available CPU cores
ndk-build -j8
```

### Step 3: Verify Output Binary
Upon successful compilation, the 64-bit ELF executable will be generated at:
```
libs/arm64-v8a/和平.sh
```
*(Note: Named with `.sh` extension per `Android.mk` configuration, but the format is a standard 64-bit ARM ELF executable).*

---

## 5. Installation & Execution

### 1. Push Executable to Device via ADB
```bash
adb push libs/arm64-v8a/和平.sh /data/local/tmp/
```

### 2. Grant Permissions and Execute with Root
```bash
adb shell
su
cd /data/local/tmp
chmod 777 和平.sh
./和平.sh
```

### 3. Select Driver IPC Mode
During launch, select the corresponding driver communication method:
* Enter `0`: Syscall Hook Mode (`ditpro 3.3`).
* Enter `1`: Raw Netlink Socket Mode (`dit 3.3`).

---

## 6. Project Documentation

* [BAO_CAO_UPDATE_OFFSET.md](BAO_CAO_UPDATE_OFFSET.md): In-depth report covering Unreal Engine 4 entity chains (GNames, UWorld, PersistentLevel, AActor, SkeletalMeshComponent, Bone Transform Matrices) for patch updates.
* [HUONG_DAN_CAI_DAT.md](HUONG_DAN_CAI_DAT.md): Step-by-step troubleshooting guide for NDK compilation issues, driver initialization errors, and input device mapping.

---

## 7. Anti-Cheat & Integrity Mechanisms Overview

Common mobile anti-cheat engines (such as Tencent ACE / `libanogs.so` / `libtersafe.so`) implement multi-layered defensive routines:

| Security Domain | Representative Offsets | Mechanism & Architecture |
| :--- | :--- | :--- |
| **Integrity & Hash Verification** | `0x495160`, `0x338AFC`, `0x3379F0`, `0x3853CC` | Anti-cheat routines scan `.text` code segments in memory and compare CRC32 / cryptographic hashes against pristine disk signatures to detect inline hooks, breakpoints, and code modifications. |
| **Environment & Hypervisor Detection** | `0x3B6E2C`, `0x2A57B8`, `0x3AE4DC` | Checks system hardware identifiers (QEMU devices, kernel properties, `/dev/` nodes, TSC timing skews) to identify unauthorized virtual machines and emulators. |
| **Telemetry & Heartbeat Sockets** | `0x4DE660` (`SoctRd`), `0x24083C` | Periodic bidirectional telemetry packets transmitting client environment health metrics to remote game security backends. Dropped or malformed packets trigger disconnects or watchdog timeouts. |
| **VTable & Control Flow Guard** | `0x3AE4DC` | Verifies virtual method table pointers to prevent virtual function hijacking on critical game engine objects. |
| **Watchdogs & Defensive Crash Traps** | `0x302D18`, `0x2191B0`, `0x239B40` | Security traps configured to force application termination (`abort()`, invalid pointer dereference) when debugger attachment or memory tampering is detected. |

---

## 8. Disclaimer

> [!WARNING]
> This repository and its contents are provided **strictly for academic research and educational purposes**, specifically analyzing Android Graphics Subsystem (Vulkan) architecture, Linux Input Subsystems, and modern game engine security patterns. Any usage for commercial game tampering or disruptive activities in online multiplayer environments is strictly prohibited.
