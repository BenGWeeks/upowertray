# upowertray

A simple, lightweight battery tray indicator for Linux using UPower. Works natively with KDE Plasma on Wayland.

## Why?

KDE Plasma's built-in battery indicator is tied to PowerDevil. If you disable PowerDevil (e.g., to let systemd-logind handle lid-close hibernate), the battery icon disappears.

Existing alternatives like `cbatticon` and `fdpowermon` use GTK and the legacy XEmbed protocol, which doesn't work on Wayland (KDE uses StatusNotifierItem).

**upowertray** solves this by:
- Using Qt6's `QSystemTrayIcon` which automatically uses StatusNotifierItem on KDE
- Reading battery data directly from UPower via DBus
- **Not** taking any inhibitor locks (won't interfere with systemd power management)

## Features

- Shows battery percentage with color-coded icon (green/yellow/red)
- Charging indicator (lightning bolt overlay)
- Tooltip with current percentage and state
- Low battery warning at 20%
- Critical battery warning at 5%
- Updates every 30 seconds
- Left-click to force update

## Dependencies

- Qt6 (Core, Gui, Widgets, DBus)
- UPower (usually pre-installed on Linux)
- CMake 3.16+
- C++17 compiler

### Ubuntu/Debian

```bash
sudo apt install qt6-base-dev libgl1-mesa-dev cmake build-essential
```

### Fedora

```bash
sudo dnf install qt6-qtbase-devel cmake gcc-c++
```

### Arch Linux

```bash
sudo pacman -S qt6-base cmake
```

## Build

```bash
git clone https://github.com/BenGWeeks/upowertray.git
cd upowertray
mkdir build && cd build
cmake ..
make
```

## Install

```bash
sudo make install
```

Or copy manually:

```bash
sudo cp upowertray /usr/local/bin/
sudo cp ../upowertray.desktop /usr/share/applications/
```

## Autostart

Copy the desktop file to your autostart directory:

```bash
cp upowertray.desktop ~/.config/autostart/
```

Or add `upowertray` to your desktop environment's startup applications.

## Verify No Inhibitors

To confirm upowertray doesn't interfere with systemd power management:

```bash
systemd-inhibit --list --mode=block
```

upowertray should NOT appear in this list.

## License

MIT License - see [LICENSE](LICENSE)
