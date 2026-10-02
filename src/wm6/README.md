# Windows Mobile 6 / HTC S730 bootstrap

This directory contains the native Windows Mobile 6 Standard bring-up for the HTC S730.

## Milestone 1: prove the phone can create and present a D3DM device

`WM6Bootstrap.cpp` is intentionally tiny. It does not depend on SDL, OpenGL ES, C++17, or the desktop/3DS build system.

It:

1. creates a 320x240 Win32/Windows Mobile window;
2. calls `Direct3DMobileCreate(D3DM_SDK_VERSION)`;
3. queries adapter 0 with `GetDeviceCaps`;
4. creates a native `IDirect3DMobileDevice`;
5. clears a native 320x240 R5G6B5 back buffer every frame;
6. calls `BeginScene` / `EndScene` / `Present`;
7. exits with the Escape key.

Microsoft documents `Direct3DMobileCreate`, `CreateDevice`, and the `IDirect3DMobileDevice` rendering/presentation methods as Windows CE 5.0+ APIs provided through `D3dm.h`, `D3dm.lib`, and `D3dmguid.lib`.

## Visual Studio 2008

Use the existing **Win32 Smart Device Project** and add:

- `src/wm6/WM6Bootstrap.cpp`

Target:

- Windows Mobile 6 Standard QVGA 320x240
- ARMV4I
- Release
- no MFC / no ATL

Link:

- `D3dm.lib`
- `D3dmguid.lib` if the linker requires it

## What success looks like

The application should open a 320x240 window and continuously clear/present an animated color field.

If `Direct3DMobileCreate` fails, the runtime is not exposed by the device.

If `CreateDevice` fails, the runtime exists but the selected presentation format/device configuration is not accepted.

If the app reaches the animated frame, we have crossed the important first boundary: **the S730 is executing our native ARMV4I application and presenting frames through a real D3DM device.**

This does not yet prove that the D3DM driver is hardware accelerated. The next milestone is a very small vertex-buffer triangle/cube test and capability dump, followed by the WM6 RenderAPI backend.

## Architecture

`PlatformConfig.h` now has `PLATFORM_WM6`. The WM6 port is deliberately isolated from `PLATFORM_PC`, `PLATFORM_3DS`, Wii, and PS2 code.

The long-term path is:

`WM6 bootstrap -> D3DM device test -> D3DM RenderAPI backend -> terrain batching -> Minecraft game loop`

The bootstrap stays in the tree as a hardware bring-up diagnostic even after the real renderer exists.
