# gammagui

A minimal GUI wrapper around `xrandr` to adjust gamma and brightness for a output.

---
**CURRENTLY ONLY SUPPORTS x11, WAYLAND SUPPORT IS PLANNED**
## Features

- Simple GUI sliders to adjust gamma and brightness for the selected output.  
- Installs user-local binaries to **~/bin** and data files to **~/.local/share**.  
- Uses a manifest file for easy uninstall.  
- No `sudo` permissions required for a install or uninstall.  
- Repository layouts using a top-level `share/` directory are mapped correctly into the XDG data directory.

---

## Quick start

1. Ensure Python and `xrandr` are installed.
2. Clone the repository, cd to the directory, make the script runnable and run the installer.
```bash
git clone https://github.com/vexalous/x11gammagui.git && cd x11gammagui && chmod +x ./install.sh && ./install.sh
```
