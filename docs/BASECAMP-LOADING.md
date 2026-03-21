# Loading YOLO in Basecamp

This document explains how to build, bundle, and install the YOLO module
into Logos Basecamp.

## What is a .lgx bundle?

A `.lgx` file is a tar.gz archive containing everything Basecamp needs to
load a module:

```
yolo/
├── manifest.json        # Bundle manifest (name, version, plugin paths)
├── metadata.json        # Headless plugin metadata
├── ui_metadata.json     # UI plugin metadata
├── yolo_plugin.so       # Headless module plugin (logoscore)
├── yolo_ui.so           # UI plugin (IComponent for logos-app)
└── qml/                 # QML interface files
    ├── MainView.qml
    ├── BoardList.qml
    ├── BoardView.qml
    ├── CreatePost.qml
    └── EventLog.qml
```

## Building the .lgx bundle

### Option A: Makefile (standard build)

```bash
# Build both plugins and package as .lgx
make bundle-lgx

# Output: dist/yolo-0.1.0.lgx
```

### Option B: Standalone script

```bash
./scripts/bundle-lgx.sh
# or specify output dir:
./scripts/bundle-lgx.sh --output /path/to/output
```

### Option C: Nix (reproducible build)

```bash
# Via Nix flake
nix build .#lgx

# Or via Makefile wrapper
make bundle-lgx-nix
```

## Installing in Basecamp

### Method 1: lgpm (Logos Package Manager) — Recommended

```bash
lgpm install dist/yolo-0.1.0.lgx
```

`lgpm` will:
- Validate the bundle manifest
- Extract the headless plugin to the modules directory
- Extract the UI plugin to the plugins directory
- Register QML files for the UI engine
- Resolve and verify dependencies

To uninstall:
```bash
lgpm remove yolo
```

### Method 2: Manual installation

Extract and copy the bundle contents to the Basecamp data directories:

```bash
# Extract the bundle
mkdir -p /tmp/yolo-install
tar xzf dist/yolo-0.1.0.lgx -C /tmp/yolo-install

# Install headless module
MODULES_DIR=~/.local/share/Logos/LogosAppNix/modules/yolo
mkdir -p "$MODULES_DIR"
cp /tmp/yolo-install/yolo/yolo_plugin.so  "$MODULES_DIR/"
cp /tmp/yolo-install/yolo/metadata.json   "$MODULES_DIR/manifest.json"

# Install UI plugin
PLUGINS_DIR=~/.local/share/Logos/LogosAppNix/plugins/yolo_ui
mkdir -p "$PLUGINS_DIR"
cp /tmp/yolo-install/yolo/yolo_ui.so      "$PLUGINS_DIR/"
cp /tmp/yolo-install/yolo/ui_metadata.json "$PLUGINS_DIR/metadata.json"

# Install QML (optional — QML is embedded in the .so via .qrc,
# but raw files allow Basecamp theme overrides)
QML_DIR=~/.local/share/Logos/LogosAppNix/plugins/yolo_ui/qml
mkdir -p "$QML_DIR"
cp /tmp/yolo-install/yolo/qml/*.qml       "$QML_DIR/"

# Cleanup
rm -rf /tmp/yolo-install
```

### Method 3: make install (development)

For development, skip the `.lgx` step and install directly:

```bash
# Install both headless + UI plugins
make install-all
```

## Directory layout after installation

```
~/.local/share/Logos/LogosAppNix/
├── modules/
│   └── yolo/
│       ├── yolo_plugin.so      # Headless plugin loaded by logoscore
│       └── manifest.json       # Module metadata
└── plugins/
    └── yolo_ui/
        ├── yolo_ui.so          # UI plugin loaded by logos-app
        ├── metadata.json       # UI plugin metadata
        └── qml/                # QML override files (optional)
            ├── MainView.qml
            ├── BoardList.qml
            ├── BoardView.qml
            ├── CreatePost.qml
            └── EventLog.qml
```

## Verifying the installation

After installing, launch Basecamp and verify:

```bash
# Start logos-app
cd ~/logos-workspace && nix run .#logos-app-poc

# Or if using the standalone binary:
logos-app
```

The YOLO module should appear in the app sidebar. You can also verify
from the command line:

```bash
# Check headless module is loadable
ls ~/.local/share/Logos/LogosAppNix/modules/yolo/yolo_plugin.so

# Check UI plugin is loadable
ls ~/.local/share/Logos/LogosAppNix/plugins/yolo_ui/yolo_ui.so

# List installed modules via lgpm
lgpm list
```

## Troubleshooting

| Problem | Fix |
|---------|-----|
| Module not appearing in sidebar | Check `manifest.json` exists in the modules dir |
| UI loads but shows blank | Ensure Qt6 Quick/Widgets are available at runtime |
| "Plugin failed to load" | Run `ldd yolo_plugin.so` to check for missing libs |
| Board features unavailable | The `sync_module` (logos-pipe) must also be installed |
| Permission denied on install | Ensure write access to `~/.local/share/Logos/` |
