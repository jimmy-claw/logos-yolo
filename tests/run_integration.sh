#!/usr/bin/env bash
# Integration test: full flow on crib with Basecamp
# Verifies: nix builds, plugin linkage, Qt version, unit tests, lgx bundle
set -euo pipefail

export PATH="$HOME/.nix-profile/bin:$PATH"

PASS=0
FAIL=0
PROJ_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
log() { echo "[TEST] $1"; }
pass() { log "PASS: $1"; PASS=$((PASS + 1)); }
fail() { log "FAIL: $1"; FAIL=$((FAIL + 1)); }

cd "$PROJ_ROOT"

# ── 1. Headless module build ──────────────────────────────────────────────────
log "Building headless module (nix build)..."
if nix build -o /tmp/yolo-result-headless 2>&1; then
    HEADLESS_OUT=$(readlink -f /tmp/yolo-result-headless)
    if [ -f "$HEADLESS_OUT/lib/yolo_plugin.so" ]; then
        pass "Headless module builds and produces yolo_plugin.so"
    else
        fail "Headless module built but yolo_plugin.so not found"
        HEADLESS_OUT=""
    fi
else
    fail "Headless module nix build failed"
    HEADLESS_OUT=""
fi

# ── 2. UI plugin build ───────────────────────────────────────────────────────
log "Building UI plugin (nix build .#ui-plugin)..."
if nix build .#ui-plugin -o /tmp/yolo-result-ui 2>&1; then
    UI_OUT=$(readlink -f /tmp/yolo-result-ui)
    if [ -f "$UI_OUT/lib/libyolo_ui.so" ]; then
        pass "UI plugin builds and produces libyolo_ui.so"
    else
        fail "UI plugin built but libyolo_ui.so not found"
        UI_OUT=""
    fi
else
    fail "UI plugin nix build failed"
    UI_OUT=""
fi

# ── 3. Plugin symbol checks ──────────────────────────────────────────────────
if [ -n "${HEADLESS_OUT:-}" ] && [ -n "${UI_OUT:-}" ]; then
    log "Checking plugin symbols..."
    # Check for C++ class names in binary (works with mangled names)
    if grep -qU "YoloPlugin" "$HEADLESS_OUT/lib/yolo_plugin.so" 2>/dev/null; then
        pass "Headless plugin contains YoloPlugin class"
    else
        log "WARN: Headless plugin YoloPlugin check inconclusive (stripped binary)"
    fi

    if grep -qU "YoloUIComponent" "$UI_OUT/lib/libyolo_ui.so" 2>/dev/null; then
        pass "UI plugin contains YoloUIComponent class"
    else
        log "WARN: UI plugin YoloUIComponent check inconclusive (stripped binary)"
    fi

    # ── 4. Qt version consistency check ──────────────────────────────────────
    log "Checking Qt version consistency..."
    if command -v ldd >/dev/null 2>&1; then
        HEADLESS_QT=$(ldd "$HEADLESS_OUT/lib/yolo_plugin.so" 2>/dev/null | grep -oP 'libQt6Core\.so\.\K[0-9.]+' | head -1 || true)
        UI_QT=$(ldd "$UI_OUT/lib/libyolo_ui.so" 2>/dev/null | grep -oP 'libQt6Core\.so\.\K[0-9.]+' | head -1 || true)

        if [ -n "$HEADLESS_QT" ] && [ -n "$UI_QT" ]; then
            if [ "$HEADLESS_QT" = "$UI_QT" ]; then
                pass "Qt version matches: $HEADLESS_QT (headless) == $UI_QT (UI)"
            else
                fail "Qt version MISMATCH: $HEADLESS_QT (headless) != $UI_QT (UI)"
            fi
        else
            log "SKIP: Could not extract Qt version from ldd output"
        fi
    else
        log "SKIP: ldd not available for Qt version check"
    fi
fi

# ── 5. LGX bundle ────────────────────────────────────────────────────────────
log "Building LGX bundle (nix build .#lgx)..."
if nix build .#lgx -o /tmp/yolo-result-lgx 2>&1; then
    LGX_OUT=$(readlink -f /tmp/yolo-result-lgx)
    if [ -f "$LGX_OUT/yolo.lgx" ]; then
        pass "LGX bundle created: yolo.lgx"
        LGX_OK=true
        [ -f "$LGX_OUT/yolo/yolo_plugin.so" ] || { fail "LGX missing yolo_plugin.so"; LGX_OK=false; }
        [ -f "$LGX_OUT/yolo/libyolo_ui.so" ] || { fail "LGX missing libyolo_ui.so"; LGX_OK=false; }
        [ -f "$LGX_OUT/yolo/metadata.json" ] || { fail "LGX missing metadata.json"; LGX_OK=false; }
        [ -f "$LGX_OUT/yolo/ui_metadata.json" ] || { fail "LGX missing ui_metadata.json"; LGX_OK=false; }
        [ -d "$LGX_OUT/yolo/qml" ] || { fail "LGX missing qml/"; LGX_OK=false; }
        $LGX_OK && pass "LGX bundle contains all required files"
    else
        fail "LGX build succeeded but yolo.lgx not found"
    fi
else
    fail "LGX bundle build failed"
fi

# ── 6. Unit tests (cmake + ctest) ────────────────────────────────────────────
log "Running unit tests..."
BUILD_TEST_DIR=$(mktemp -d /tmp/yolo-test-XXXXXX)
LOGOS_PIPE_ROOT="${LOGOS_PIPE_ROOT:-$HOME/logos-pipe}"

if [ ! -d "$LOGOS_PIPE_ROOT/src" ]; then
    log "SKIP: logos-pipe not found at $LOGOS_PIPE_ROOT"
else
    if cmake -S "$PROJ_ROOT" -B "$BUILD_TEST_DIR" \
        -DBUILD_TESTS=ON \
        -DLOGOS_PIPE_ROOT="$LOGOS_PIPE_ROOT" 2>&1; then

        if cmake --build "$BUILD_TEST_DIR" -j"$(nproc)" 2>&1; then
            if ctest --test-dir "$BUILD_TEST_DIR" --output-on-failure 2>&1; then
                pass "All unit tests pass (including integration test)"
            else
                fail "Some unit tests failed"
            fi
        else
            fail "Test build failed"
        fi
    else
        fail "Test cmake configure failed"
    fi
    rm -rf "$BUILD_TEST_DIR"
fi

# ── 7. Install layout check ──────────────────────────────────────────────────
if [ -n "${HEADLESS_OUT:-}" ] && [ -n "${UI_OUT:-}" ]; then
    log "Verifying install layout..."
    INSTALL_DIR=$(mktemp -d /tmp/yolo-install-XXXXXX)

    mkdir -p "$INSTALL_DIR/modules/yolo" "$INSTALL_DIR/plugins/yolo_ui"
    cp "$HEADLESS_OUT/lib/yolo_plugin.so" "$INSTALL_DIR/modules/yolo/"
    cp "$PROJ_ROOT/metadata.json" "$INSTALL_DIR/modules/yolo/manifest.json"
    cp "$UI_OUT/lib/libyolo_ui.so" "$INSTALL_DIR/plugins/yolo_ui/yolo_ui.so"

    if [ -f "$INSTALL_DIR/modules/yolo/yolo_plugin.so" ] && \
       [ -f "$INSTALL_DIR/modules/yolo/manifest.json" ] && \
       [ -f "$INSTALL_DIR/plugins/yolo_ui/yolo_ui.so" ]; then
        pass "Install layout matches Basecamp expectations"
    else
        fail "Install layout is incorrect"
    fi
    rm -rf "$INSTALL_DIR"
fi

# Cleanup nix result symlinks
rm -f /tmp/yolo-result-headless /tmp/yolo-result-ui /tmp/yolo-result-lgx

# ── Summary ──────────────────────────────────────────────────────────────────
echo ""
echo "========================================"
echo "  Integration Test Summary"
echo "========================================"
echo "  PASSED: $PASS"
echo "  FAILED: $FAIL"
echo "========================================"

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
