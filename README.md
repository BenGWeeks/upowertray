<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/logo.svg">
    <source media="(prefers-color-scheme: light)" srcset="docs/logo.svg">
    <img alt="upowertray" src="docs/logo.svg" width="100%">
  </picture>
</p>

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
- Configurable low/critical battery warning thresholds
- Settings dialog showing system power settings (UPower + logind)
- Updates every 30 seconds
- Left-click to open settings dialog

## Configuration

Left-click the tray icon to view power settings. The dialog shows:
- Current battery status
- Notification thresholds (from UPower)
- Battery critical action (from UPower)
- Lid close action (from systemd-logind)

Settings are read from system config files:
- `/etc/UPower/UPower.conf` - Battery thresholds and actions
- `/etc/systemd/logind.conf` - Lid close behavior

## Icon Preview

![Battery icon states](docs/icon-preview.svg)

The icon changes color based on battery level:
- **Green**: Above 20%
- **Yellow**: 10-20%
- **Red**: Below 10%
- **Lightning bolt**: Charging

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

## Sleep Modes Explained

| Mode | What happens | Battery drain | Resume time |
|------|--------------|---------------|-------------|
| **Suspend** | RAM stays powered | 5-10%/day | Instant |
| **Hibernate** | RAM → disk, power off | 0% | 10-30s |
| **HybridSleep** | RAM → disk, then suspend | ~2-3%/day | Instant* |

\* HybridSleep resumes instantly if battery is OK, or from disk if battery died.

### Recommended Configuration

| Trigger | Action | Why |
|---------|--------|-----|
| Lid close | Hibernate | 0% drain when laptop is put away |
| Battery critical | HybridSleep | Quick wake if plugged in, safe if battery dies |

See [docs/HIBERNATE.adoc](docs/HIBERNATE.adoc) for setup instructions.

## Verify No Inhibitors

To confirm upowertray doesn't interfere with systemd power management:

```bash
systemd-inhibit --list --mode=block
```

upowertray should NOT appear in this list.

## Documentation

- [Installation Guide](docs/INSTALLATION.adoc)
- [Hibernation Setup](docs/HIBERNATE.adoc)
- [Troubleshooting](docs/TROUBLESHOOTING.adoc)
- [Solution Architecture](docs/SOLUTION_ARCHITECTURE.adoc)
- [Packaging Guide](docs/PACKAGE.adoc)

## License

MIT License - see [LICENSE](LICENSE)
