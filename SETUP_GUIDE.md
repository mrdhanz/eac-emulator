# Setup Guide — Linux Virtual Machine (VMware)

This guide walks through configuring a Linux virtual machine using VMware to run the Linux / Proton side of the emulator while running your game client on your Windows host.

---

## 1. Building the Project

Build both the Windows client DLL and the Linux server DLL using Visual Studio 2022:

1. Open [EOSEmulator.sln](file:///e:/Project/eac-emulator/EOSEmulator.sln) in Visual Studio 2022.
2. Select **Release | x64**.
3. Build the solution (`Ctrl+Shift+B` or run `msbuild EOSEmulator.sln /p:Configuration=Release /p:Platform=x64`).
4. Output binaries are generated in `bin\Release\`:
   - `EOSSDK-Win64-Shipping.dll` (for Windows client)
   - `version.dll` (for Linux VM server)

---

## 2. Virtual Machine Setup (VMware)

VMware Workstation or VMware Player is recommended for hosting the Linux VM.

### Change the MAC Address
In your VM settings:
1. Open the **Network Adapter** tab and click **Advanced**.
2. Change the MAC address to a custom address such as `00:0C:29:00:00:01` (avoid prefixes starting with `00:50`).

### Bypass Anti-VM Detection
To prevent EasyAntiCheat from detecting the hypervisor environment:
1. Ensure the VM is powered off.
2. Open the VM's `.vmx` configuration file in a text editor.
3. Append the following configuration lines to the bottom of the file:

```ini
hypervisor.cpuid.v0 = "FALSE"
board-id.reflectHost = "TRUE"
hw.model.reflectHost = "TRUE"
serialNumber.reflectHost = "TRUE"
smbios.reflectHost = "TRUE"
SMBIOS.noOEMStrings = "TRUE"
isolation.tools.getPtrLocation.disable = "TRUE"
isolation.tools.setPtrLocation.disable = "TRUE"
isolation.tools.setVersion.disable = "TRUE"
isolation.tools.getVersion.disable = "TRUE"
monitor_control.disable_directexec = "TRUE"
monitor_control.disable_chksimd = "TRUE"
monitor_control.disable_ntreloc = "TRUE"
monitor_control.disable_selfmod = "TRUE"
monitor_control.disable_reloc = "TRUE"
monitor_control.disable_btinout = "TRUE"
monitor_control.disable_btmemspace = "TRUE"
monitor_control.disable_btpriv = "TRUE"
monitor_control.disable_btseg = "TRUE"
monitor_control.restrict_backdoor = "TRUE"
scsi0:0.productID = "Tencent SSD"
scsi0:0.vendorID = "Tencent"
```

Save the file and power on the VM.

---

## 3. Prepare Linux Side

### Install Steam & Game
1. Install Steam on your Linux distribution (Linux Mint, Ubuntu, Arch, etc.).
2. In Steam Settings -> **Compatibility**, check **Enable Steam Play for all other titles** (select **Proton Experimental** or **Proton Hotfix**).
3. Install the target game (e.g., SpiritVale or VRChat) and run it at least once.

### Place the Bootstrapper DLL
Copy `version.dll` into the game directory next to the game executable:
```bash
# Path typically: ~/.steam/steam/steamapps/common/<GameFolder>/
cp /path/to/version.dll ~/.steam/steam/steamapps/common/<GameFolder>/
```

### Configure Proton Environment via Protontricks
1. Install Protontricks:
   ```bash
   flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo
   flatpak install com.github.Matoking.protontricks --system
   ```
2. Launch Protontricks GUI for your game:
   ```bash
   flatpak run com.github.Matoking.protontricks <SteamAppId> --gui
   ```
3. Select **Select the default wineprefix** -> **OK**.
4. Configure DLL Overrides:
   - Select **Run winecfg**.
   - Navigate to the **Libraries** tab.
   - Under *New override for library*, type `version` and click **Add**.
   - Ensure `version` is set to `(native, builtin)` and click **OK**.
5. Install Visual C++ Redistributable:
   - Download the [Visual C++ Redistributable All-in-One](https://www.techpowerup.com/download/visual-c-redistributable-runtime-package-all-in-one/).
   - In Protontricks, select **Run Uninstaller** -> click **Install** -> choose the `.bat` installer script from the extracted package.

---

## 4. Prepare Windows Side

1. Copy `EOSSDK-Win64-Shipping.dll` to the game's plugin directory on Windows:
   ```text
   <GameDirectory>\<GameName>_Data\Plugins\x86_64\EOSSDK-Win64-Shipping.dll
   ```
   *(e.g., `SpiritVale_Data\Plugins\x86_64\EOSSDK-Win64-Shipping.dll`)*

2. Launch `<GameExecutable>.exe` directly on Windows once to generate `config.json`.

3. Edit `config.json` with the local IP address of your Linux VM:
   ```json
   {
       "ip": "192.168.146.129",
       "ports": {
           "http": 7778,
           "tcp": 7777
       }
   }
   ```
   *(Replace `192.168.146.129` with your Linux VM's actual IP address found using `ip addr` in Linux).*

---

## 5. Launch Workflow

1. Start the game on the Linux VM (this starts the emulator HTTP and WebSocket servers).
2. Launch the Windows game executable directly.
3. The Windows client connects to the Linux VM proxy for anticheat verification and session management.
