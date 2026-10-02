# Windows Mobile 6 / Direct3D Mobile probe

This directory contains the first native Windows Mobile 6 Standard graphics probe for the HTC S730 port.

## Visual Studio 2008

Add `src/wm6/D3dmProbe.cpp` to a **Win32 Smart Device Project** targeting:

- Windows Mobile 6 Standard QVGA 320x240
- ARMV4I
- Release

The source intentionally uses only the native Direct3D Mobile API. Link against the WM6 SDK's `D3dm.lib` (and, if the linker requests it, `D3dmguid.lib`).

The probe first calls `Direct3DMobileCreate(D3DM_SDK_VERSION)` and then queries adapter 0 with `GetDeviceCaps`. Microsoft documents these APIs as part of Direct3D Mobile on Windows CE 5.0+.

## What the result tells us

- If `Direct3DMobileCreate` fails, the phone does not expose a usable D3DM runtime to the application.
- If it succeeds, the message box prints driver capabilities such as maximum texture size and vertex/raster capabilities.
- This is deliberately **not** the Minecraft renderer yet. We want to establish that D3DM is actually available on the S730 before replacing the 3DS PICA200 backend.

The next probe should create a 320x240 D3DM device and render a textured rotating cube. That test is the important one for determining whether D3DM is hardware accelerated enough for the full port.
