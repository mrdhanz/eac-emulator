# EasyAntiCheat & EOS Emulator

An EasyAntiCheat (EAC) and Epic Online Services (EOS) emulation proxy designed to redirect EOS and anticheat communications between a Windows game client and a Linux / Proton environment.

## Architecture

The project consists of two primary components built from [EOSEmulator.sln](file:///e:/Project/eac-emulator/EOSEmulator.sln):

1. **`EOSSDK-Win64-Shipping.dll`** (Windows Client):
   Replaces the game's native EOS SDK in `<Game>_Data\Plugins\x86_64\EOSSDK-Win64-Shipping.dll`. Intercepts platform creation, connect login, and anticheat packet exchanges, proxying requests over HTTP (port 7778) and WebSocket (port 7777) to the Linux/Proton emulator server.
2. **`version.dll`** (Linux / Proton Server):
   A proxy DLL loaded via `WINEDLLOVERRIDES` in Proton/Wine alongside the target game executable. Hosts the HTTP/WebSocket servers and forwards anticheat and platform session state to the real Linux EOS environment.

Target game process and EOS SDK module paths are automatically detected at runtime, eliminating the need to hardcode game-specific executables or directories.

# Getting started
## Cloning

```
git clone https://github.com/mrdhanz/eac-emulator.git
```

## Building

### Requirements
- Windows 10/11
- Visual Studio 2022 (v143 toolset with **Desktop development with C++**)
- MSBuild (included with Visual Studio 2022)

### Build with Visual Studio
1. Open [EOSEmulator.sln](file:///e:/Project/eac-emulator/EOSEmulator.sln) in Visual Studio 2022.
2. Select **Release** and **x64** in the configuration dropdown.
3. Build the solution (**Build -> Build Solution** or `Ctrl+Shift+B`).

### Build with Command Line (MSBuild)
```powershell
msbuild EOSEmulator.sln /p:Configuration=Release /p:Platform=x64
```

Build outputs will be generated in:
- `bin\Release\EOSSDK-Win64-Shipping.dll`
- `bin\Release\version.dll`

## Setup Guides

For detailed environment setup instructions, refer to:
- [SETUP_GUIDE_WSL.md](./SETUP_GUIDE_WSL.md) — Recommended guide for WSL2 (Ubuntu) with Proton.
- [SETUP_GUIDE.md](./SETUP_GUIDE.md) — Guide for standalone Linux Virtual Machines (VMware).

## Libraries Used

- [MinHook](https://github.com/TsudaKageyu/minhook) - API hooking library
- [Plog](https://github.com/SergiusTheBest/plog) - C++ logging library
- [libhv](https://github.com/ithewei/libhv) - Cross-platform event loop and HTTP/WebSocket library
- [JSON for Modern C++](https://github.com/nlohmann/json) - JSON parser
