# upowertray

A Qt6 battery tray indicator for KDE Plasma on Wayland.

## Project Structure

```
upowertray/
├── CMakeLists.txt      # CMake build configuration
├── src/
│   ├── main.cpp        # Application entry point
│   ├── batterytray.h   # BatteryTray class header
│   └── batterytray.cpp # Main implementation
├── upowertray.desktop  # Desktop entry for autostart
├── README.md           # User documentation
└── LICENSE             # MIT License
```

## Key Components

### BatteryTray class (`src/batterytray.cpp`)
- Uses `QSystemTrayIcon` for StatusNotifierItem protocol (Wayland-compatible)
- Communicates with UPower via DBus at `org.freedesktop.UPower`
- Battery device path: `/org/freedesktop/UPower/devices/battery_BAT0`
- Updates every 30 seconds via `QTimer`
- Draws battery icon programmatically with `QPainter`

### UPower DBus Properties
- `Percentage` (double): 0-100
- `State` (uint32): 1=charging, 2=discharging, 3=empty, 4=fully-charged

### Constants
- `LOW_BATTERY_THRESHOLD`: 20%
- `CRITICAL_BATTERY_THRESHOLD`: 5%
- `UPDATE_INTERVAL_MS`: 30000 (30 seconds)

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Dependencies

- Qt6 (Core, Gui, Widgets, DBus)
- CMake 3.16+
- C++17 compiler

## Why This Exists

KDE Plasma's battery indicator requires PowerDevil. Disabling PowerDevil (to allow systemd-logind to handle lid-close hibernate) removes the battery icon. GTK alternatives don't work on Wayland due to XEmbed vs StatusNotifierItem protocol mismatch.
