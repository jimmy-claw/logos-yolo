#!/usr/bin/env bash
# bundle-lgx.sh — Package YOLO module as .lgx for Basecamp
#
# Usage:
#   ./scripts/bundle-lgx.sh [--output DIR]
#
# Builds headless + UI plugins, then packages them with metadata and QML
# into a .lgx archive (tar.gz) ready for Basecamp installation.
#
# Prerequisites:
#   - Logos SDK (LOGOS_CPP_SDK_ROOT or nix store packages)
#   - Logos liblogos (LOGOS_LIBLOGOS_ROOT or nix store packages)
#   - Qt6 (Core, Qml, Quick, Widgets, RemoteObjects, QuickWidgets)
#   - logos-pipe source tree (LOGOS_PIPE_ROOT, default: ~/logos-pipe)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
OUTPUT_DIR="${PROJECT_DIR}/dist"
VERSION="0.1.0"
BUNDLE_NAME="yolo"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [--output DIR]"
            echo ""
            echo "Builds and packages YOLO as .lgx for Basecamp."
            echo ""
            echo "Options:"
            echo "  --output DIR   Output directory (default: dist/)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

echo "=== YOLO .lgx Bundle Builder ==="
echo "Project: ${PROJECT_DIR}"
echo "Output:  ${OUTPUT_DIR}"
echo ""

# ── Step 1: Build headless module plugin ────────────────────────────────────

echo "[1/4] Building headless module plugin..."
make -C "${PROJECT_DIR}" build-module 2>&1 | tail -5
HEADLESS_SO="${PROJECT_DIR}/build-module/yolo_plugin.so"
if [[ ! -f "$HEADLESS_SO" ]]; then
    echo "ERROR: headless plugin build failed — yolo_plugin.so not found" >&2
    exit 1
fi
echo "      -> yolo_plugin.so built"

# ── Step 2: Build UI plugin ────────────────────────────────────────────────

echo "[2/4] Building UI plugin..."
make -C "${PROJECT_DIR}" build-ui-plugin 2>&1 | tail -5
UI_SO="${PROJECT_DIR}/build-ui-plugin/libyolo_ui.so"
if [[ ! -f "$UI_SO" ]]; then
    echo "ERROR: UI plugin build failed — libyolo_ui.so not found" >&2
    exit 1
fi
echo "      -> libyolo_ui.so built"

# ── Step 3: Assemble staging directory ──────────────────────────────────────

echo "[3/4] Assembling bundle..."
STAGING=$(mktemp -d)
trap 'rm -rf "$STAGING"' EXIT

BUNDLE_DIR="${STAGING}/${BUNDLE_NAME}"
mkdir -p "${BUNDLE_DIR}/qml"

# Plugins
cp "$HEADLESS_SO" "${BUNDLE_DIR}/yolo_plugin.so"
cp "$UI_SO"       "${BUNDLE_DIR}/yolo_ui.so"

# Metadata
cp "${PROJECT_DIR}/metadata.json"    "${BUNDLE_DIR}/metadata.json"
cp "${PROJECT_DIR}/ui_metadata.json" "${BUNDLE_DIR}/ui_metadata.json"

# QML files (raw, not embedded in .qrc — Basecamp loads these directly)
for qml_file in "${PROJECT_DIR}"/qml/*.qml; do
    cp "$qml_file" "${BUNDLE_DIR}/qml/"
done

# Bundle manifest (machine-readable summary for lgpm)
cat > "${BUNDLE_DIR}/manifest.json" <<MANIFEST
{
  "name": "${BUNDLE_NAME}",
  "version": "${VERSION}",
  "description": "YOLO community board — federated social posting for Logos Basecamp",
  "author": "jimmy-claw",
  "type": "module",
  "category": "social",
  "plugins": {
    "headless": "yolo_plugin.so",
    "ui": "yolo_ui.so"
  },
  "qml": "qml/",
  "dependencies": [],
  "manifestVersion": "0.1.0"
}
MANIFEST

echo "      -> Staged: plugins, metadata, QML, manifest"

# ── Step 4: Create .lgx archive ────────────────────────────────────────────

echo "[4/4] Creating .lgx archive..."
mkdir -p "${OUTPUT_DIR}"
LGX_PATH="${OUTPUT_DIR}/${BUNDLE_NAME}-${VERSION}.lgx"
tar czf "$LGX_PATH" -C "$STAGING" "$BUNDLE_NAME"

# Print summary
LGX_SIZE=$(du -h "$LGX_PATH" | cut -f1)
echo ""
echo "=== Bundle complete ==="
echo "  Archive: ${LGX_PATH}"
echo "  Size:    ${LGX_SIZE}"
echo ""
echo "Contents:"
tar tzf "$LGX_PATH" | sed 's/^/  /'
echo ""
echo "Install with:"
echo "  lgpm install ${LGX_PATH}"
echo "  — or —"
echo "  cp ${LGX_PATH} ~/.local/share/Logos/LogosAppNix/bundles/"
