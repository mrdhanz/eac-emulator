# Setup Guide — WSL2 with Proton

This guide explains how to set up and run the EOS / EAC Emulator using Windows Subsystem for Linux (WSL2) with Proton and your Windows host.

---

## 1. Building the Project

Before setting up the environments, build both components using Visual Studio 2022 on Windows:

1. Open [EOSEmulator.sln](file:///e:/Project/eac-emulator/EOSEmulator.sln) in Visual Studio 2022.
2. Select **Release | x64**.
3. Build the solution (`Ctrl+Shift+B` or run `msbuild EOSEmulator.sln /p:Configuration=Release /p:Platform=x64`).
4. Output binaries will be in the `bin\Release\` folder:
   - `EOSSDK-Win64-Shipping.dll` (for Windows client)
   - `version.dll` (for Linux / WSL Proton host)

---

## 2. WSL Environment Setup

Ensure WSL2 with Ubuntu (22.04 or 24.04) is installed on your Windows machine.

### Install Steam & Dependencies in WSL
Open your WSL terminal and execute:

```bash
# Download proton launch helper
wget https://gist.githubusercontent.com/thingsiplay/3a933f557277906dc6b0e03ec8df5dbd/raw/b406b32604dfb63b83d80f5bbaafd80d09f69822/proton -O proton.sh
chmod +x proton.sh

# Enable 32-bit architecture and install Steam
sudo add-apt-repository multiverse
sudo dpkg --add-architecture i386
sudo apt update
sudo apt upgrade -y
sudo apt install steam -y
steam
```

### Configure Steam & Install Target Game
1. In Steam on WSL, go to **Steam -> Settings -> Compatibility**.
2. Check **Enable Steam Play for supported titles** and **Enable Steam Play for all other titles** (select **Proton Experimental** or **Proton Hotfix**).
3. Install your target game (e.g. SpiritVale or VRChat) and run it once to initialize the Proton prefix.

---

## 3. Configure the Linux / WSL Side

### Place the Bootstrapper DLL
Copy the built `version.dll` from your Windows build output into the target game directory on WSL:
```bash
# Path typically: ~/.steam/steam/steamapps/common/<GameFolder>/
cp /mnt/e/Project/eac-emulator/bin/Release/version.dll ~/.steam/steam/steamapps/common/<GameFolder>/
```

### Configure Proton Launch Script
Edit `~/proton.sh` to match your installed Proton version and Steam directory:
```bash
proton_version="Proton Hotfix" # or "Proton Experimental"
client_dir="$HOME/.steam/steam"
```

### Launch the Linux Server Component
Launch the game inside WSL with Wine DLL override enabled so Proton loads our `version.dll`:
```bash
WINEDLLOVERRIDES="version.dll=n,b" ~/proton.sh ~/.steam/steam/steamapps/common/<GameFolder>/<GameExecutable>.exe
```
> [!TIP]
> You can create a bash script (e.g. `launch_emulator.sh`) to automate this command.

Once running, `version.dll` starts an HTTP server on port 7778 and a WebSocket server on port 7777 to handle forwarded EOS API and EAC requests.

---

## 4. Configure the Windows Client

1. Copy the built `EOSSDK-Win64-Shipping.dll` to your Windows game installation's plugin directory:
   ```text
   <GameDirectory>\<GameName>_Data\Plugins\x86_64\EOSSDK-Win64-Shipping.dll
   ```
   *(e.g., `SpiritVale_Data\Plugins\x86_64\EOSSDK-Win64-Shipping.dll` or `VRChat_Data\Plugins\x86_64\EOSSDK-Win64-Shipping.dll`)*

2. Launch `<GameExecutable>.exe` directly on Windows once. This will generate a default configuration file named `config.json` next to the executable.

3. Open `config.json` in a text editor and ensure the IP address points to your WSL instance:
   ```json
   {
       "ip": "127.0.0.1",
       "ports": {
           "http": 7778,
           "tcp": 7777
       }
   }
   ```
   > [!NOTE]
   > With WSL2 localhost forwarding (default), `127.0.0.1` connects directly to WSL services. If you have custom networking or NAT enabled, use the WSL IP obtained via `ip addr show eth0`.

---

## 5. Usage Workflow

1. Start the game via `~/proton.sh` in WSL (starts proxy servers).
2. Launch the Windows game client directly.
3. The Windows client connects to the WSL server and passes EAC authentication seamlessly.
