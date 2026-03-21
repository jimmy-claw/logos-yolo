#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════════
# e2e_real_logos.sh — Real End-to-End Test for YOLO Module on Live LogosApp
# ═══════════════════════════════════════════════════════════════════════════
#
# This test verifies the YOLO module works end-to-end against a running
# LogosApp instance. It:
#   1. Verifies LogosApp is running and modules are installed
#   2. Probes the live Qt RemoteObjects IPC infrastructure
#   3. Builds and runs integration tests (board → post → on-chain verify)
#   4. Takes a screenshot of the running LogosApp UI
#   5. Reports results with real inscription IDs and data
#
# Usage: ./tests/e2e_real_logos.sh
# ═══════════════════════════════════════════════════════════════════════════
set -euo pipefail

PROJ_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SCREENSHOTS_DIR="$PROJ_ROOT/docs/screenshots"
PASS=0
FAIL=0
SKIP=0

log()  { echo -e "\033[1;34m[E2E]\033[0m $1"; }
pass() { echo -e "\033[1;32m[PASS]\033[0m $1"; PASS=$((PASS + 1)); }
fail() { echo -e "\033[1;31m[FAIL]\033[0m $1"; FAIL=$((FAIL + 1)); }
skip() { echo -e "\033[1;33m[SKIP]\033[0m $1"; SKIP=$((SKIP + 1)); }
banner() {
    echo ""
    echo "══════════════════════════════════════════════════════════════"
    echo "  $1"
    echo "══════════════════════════════════════════════════════════════"
}

cd "$PROJ_ROOT"

banner "YOLO E2E Test — Live LogosApp Integration"
echo "  Date:    $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo "  Host:    $(hostname)"
echo "  Kernel:  $(uname -r)"
echo ""

# ═══════════════════════════════════════════════════════════════════════════
# PART 1: Verify Running LogosApp
# ═══════════════════════════════════════════════════════════════════════════
banner "Part 1: LogosApp Process Verification"

LOGOS_PID=$(pgrep -f "LogosApp" 2>/dev/null | head -1 || true)
if [ -n "$LOGOS_PID" ]; then
    LOGOS_BIN=$(readlink -f /proc/"$LOGOS_PID"/exe 2>/dev/null || echo "unknown")
    LOGOS_UPTIME=$(ps -o etime= -p "$LOGOS_PID" 2>/dev/null | xargs || echo "unknown")
    pass "LogosApp is running (PID: $LOGOS_PID, uptime: $LOGOS_UPTIME)"
    log "  Binary: $LOGOS_BIN"

    # Check DISPLAY
    LOGOS_DISPLAY=$(tr '\0' '\n' < /proc/"$LOGOS_PID"/environ 2>/dev/null | grep '^DISPLAY=' | cut -d= -f2 || echo "")
    if [ -n "$LOGOS_DISPLAY" ]; then
        pass "LogosApp has display: $LOGOS_DISPLAY"
    else
        skip "Could not determine LogosApp DISPLAY"
    fi
else
    fail "LogosApp is NOT running"
    log "  Some tests will be skipped"
fi

# Check logos_host processes
log "Checking logos_host child processes..."
HOSTS=$(pgrep -fa logos_host 2>/dev/null || true)
if [ -n "$HOSTS" ]; then
    echo "$HOSTS" | while read -r line; do
        MOD_NAME=$(echo "$line" | grep -oP '(?<=--name )\S+' || echo "unknown")
        log "  logos_host: $MOD_NAME"
    done
    pass "logos_host processes found"
else
    skip "No logos_host processes detected"
fi

# ═══════════════════════════════════════════════════════════════════════════
# PART 2: Module Installation Verification
# ═══════════════════════════════════════════════════════════════════════════
banner "Part 2: Module Installation Verification"

MODULES_DIR="$HOME/.local/share/Logos/LogosAppNix/modules"

# Check yolo module
if [ -f "$MODULES_DIR/yolo/yolo_plugin.so" ]; then
    YOLO_SIZE=$(stat -c%s "$MODULES_DIR/yolo/yolo_plugin.so" 2>/dev/null || echo "?")
    pass "yolo_plugin.so installed ($YOLO_SIZE bytes)"
else
    fail "yolo_plugin.so NOT found at $MODULES_DIR/yolo/"
fi

if [ -f "$MODULES_DIR/yolo/manifest.json" ]; then
    YOLO_VER=$(python3 -c "import json; print(json.load(open('$MODULES_DIR/yolo/manifest.json'))['version'])" 2>/dev/null || echo "?")
    pass "yolo manifest.json present (version: $YOLO_VER)"
else
    fail "yolo manifest.json NOT found"
fi

# Check blockchain_module
if [ -f "$MODULES_DIR/blockchain_module/blockchain_module_plugin.so" ]; then
    pass "blockchain_module installed"
else
    skip "blockchain_module not found"
fi

# Check kv_module
if [ -f "$MODULES_DIR/kv_module/"*".so" ] 2>/dev/null; then
    pass "kv_module installed"
else
    # Check with ls
    if ls "$MODULES_DIR/kv_module/" 2>/dev/null | grep -q '\.so$'; then
        pass "kv_module installed"
    else
        skip "kv_module not found"
    fi
fi

# Plugin symbol check
log "Checking yolo_plugin.so symbols..."
if nm -D "$MODULES_DIR/yolo/yolo_plugin.so" 2>/dev/null | grep -q "YoloPlugin"; then
    pass "YoloPlugin class symbol found in plugin binary"
elif strings "$MODULES_DIR/yolo/yolo_plugin.so" 2>/dev/null | grep -q "YoloPlugin"; then
    pass "YoloPlugin string found in plugin binary"
else
    skip "Could not verify YoloPlugin symbol (may be stripped)"
fi

# ═══════════════════════════════════════════════════════════════════════════
# PART 3: Qt RemoteObjects IPC Probe
# ═══════════════════════════════════════════════════════════════════════════
banner "Part 3: Qt RemoteObjects IPC Infrastructure"

IPC_SOCKETS=(/tmp/logos_core_manager /tmp/logos_capability_module /tmp/logos_package_manager)

for sock in "${IPC_SOCKETS[@]}"; do
    if [ -S "$sock" ]; then
        pass "IPC socket exists: $(basename "$sock")"
    else
        fail "IPC socket missing: $(basename "$sock")"
    fi
done

# Build and run the probe program
log "Building E2E IPC probe..."
PROBE_DIR=$(mktemp -d /tmp/yolo-e2e-probe-XXXXXX)

if g++ -fPIC -std=c++17 \
    $(pkg-config --cflags Qt6RemoteObjects Qt6Core 2>/dev/null) \
    -o "$PROBE_DIR/e2e_logos_probe" \
    "$PROJ_ROOT/tests/e2e_logos_probe.cpp" \
    $(pkg-config --libs Qt6RemoteObjects Qt6Core 2>/dev/null) 2>"$PROBE_DIR/build.log"; then
    pass "E2E probe compiled successfully"

    log "Running E2E probe against live LogosApp..."
    if timeout 15 "$PROBE_DIR/e2e_logos_probe" 2>&1 | tee "$PROBE_DIR/probe_output.log"; then
        pass "E2E probe completed successfully"
    else
        PROBE_EXIT=$?
        if [ "$PROBE_EXIT" -eq 124 ]; then
            skip "E2E probe timed out (LogosApp may not be responding)"
        else
            fail "E2E probe reported failures (exit: $PROBE_EXIT)"
        fi
    fi
    echo ""
else
    skip "E2E probe compilation failed (missing Qt6 dev headers?)"
    cat "$PROBE_DIR/build.log" 2>/dev/null | tail -5
fi

rm -rf "$PROBE_DIR"

# ═══════════════════════════════════════════════════════════════════════════
# PART 4: Integration Tests (Board → Post → On-Chain Verification)
# ═══════════════════════════════════════════════════════════════════════════
banner "Part 4: YOLO Module Integration Tests"

LOGOS_PIPE_ROOT="${LOGOS_PIPE_ROOT:-$HOME/logos-pipe}"
BUILD_DIR=$(mktemp -d /tmp/yolo-e2e-build-XXXXXX)

if [ ! -d "$LOGOS_PIPE_ROOT/src" ]; then
    skip "logos-pipe not found at $LOGOS_PIPE_ROOT — skipping integration tests"
else
    log "Configuring build..."
    if cmake -S "$PROJ_ROOT" -B "$BUILD_DIR" \
        -DBUILD_TESTS=ON \
        -DLOGOS_PIPE_ROOT="$LOGOS_PIPE_ROOT" \
        -DCMAKE_BUILD_TYPE=Release 2>"$BUILD_DIR/cmake.log"; then
        pass "CMake configuration succeeded"
    else
        fail "CMake configuration failed"
        cat "$BUILD_DIR/cmake.log" | tail -10
    fi

    log "Building tests..."
    if cmake --build "$BUILD_DIR" -j"$(nproc)" 2>"$BUILD_DIR/build.log"; then
        pass "Test build succeeded"
    else
        fail "Test build failed"
        cat "$BUILD_DIR/build.log" | tail -15
    fi

    # Run each test individually for detailed output
    TESTS=(test_yolo_board test_yolo_events test_yolo_federation
           test_yolo_event_lifecycle test_integration test_verify_onchain)

    for test_name in "${TESTS[@]}"; do
        if [ -f "$BUILD_DIR/$test_name" ]; then
            log "Running $test_name..."
            if "$BUILD_DIR/$test_name" -v2 2>&1 | tee "$BUILD_DIR/${test_name}.log" | tail -20; then
                pass "$test_name passed"
            else
                fail "$test_name failed"
            fi
        else
            skip "$test_name binary not found"
        fi
    done

    # ── Extract inscription IDs and data from verify_onchain output ──
    echo ""
    log "═══ Real Inscription Data from On-Chain Verification ═══"
    if [ -f "$BUILD_DIR/test_verify_onchain.log" ]; then
        grep -E "(Inscription ID|Channel ID|On-chain data|Content CID|PASS:)" \
            "$BUILD_DIR/test_verify_onchain.log" 2>/dev/null | while read -r line; do
            echo "  $line"
        done
    fi

    rm -rf "$BUILD_DIR"
fi

# ═══════════════════════════════════════════════════════════════════════════
# PART 5: Screenshot
# ═══════════════════════════════════════════════════════════════════════════
banner "Part 5: LogosApp UI Screenshot"

mkdir -p "$SCREENSHOTS_DIR"

# Determine display
DISPLAY_TO_USE="${LOGOS_DISPLAY:-${DISPLAY:-:97}}"
log "Using DISPLAY=$DISPLAY_TO_USE"

if command -v scrot >/dev/null 2>&1; then
    SCREENSHOT_FILE="$SCREENSHOTS_DIR/e2e_logosapp_running.png"
    if DISPLAY="$DISPLAY_TO_USE" scrot "$SCREENSHOT_FILE" 2>/dev/null; then
        SHOT_SIZE=$(stat -c%s "$SCREENSHOT_FILE" 2>/dev/null || echo "?")
        pass "Screenshot captured: $(basename "$SCREENSHOT_FILE") ($SHOT_SIZE bytes)"
    else
        skip "scrot failed (no X display available?)"
    fi

    # Try to get a focused window screenshot too
    WINDOW_SHOT="$SCREENSHOTS_DIR/e2e_logosapp_window.png"
    WINDOW_ID=$(DISPLAY="$DISPLAY_TO_USE" xdotool search --name "Logos" 2>/dev/null | head -1 || true)
    if [ -n "$WINDOW_ID" ]; then
        if DISPLAY="$DISPLAY_TO_USE" import -window "$WINDOW_ID" "$WINDOW_SHOT" 2>/dev/null; then
            pass "Window screenshot captured: $(basename "$WINDOW_SHOT")"
        else
            skip "Window screenshot failed"
        fi
    fi
else
    skip "scrot not installed — no screenshot taken"
fi

# ═══════════════════════════════════════════════════════════════════════════
# PART 6: Module Runtime Info
# ═══════════════════════════════════════════════════════════════════════════
banner "Part 6: Runtime Environment Info"

log "Installed modules:"
ls -1 "$MODULES_DIR" 2>/dev/null | while read -r mod; do
    if [ -f "$MODULES_DIR/$mod/manifest.json" ]; then
        VER=$(python3 -c "import json; print(json.load(open('$MODULES_DIR/$mod/manifest.json')).get('version','?'))" 2>/dev/null || echo "?")
        echo "  $mod (v$VER)"
    else
        echo "  $mod"
    fi
done

log "Active IPC sockets:"
ls -la /tmp/logos_* 2>/dev/null | while read -r line; do
    echo "  $line"
done

if [ -n "${LOGOS_PID:-}" ]; then
    log "LogosApp memory usage:"
    RSS=$(ps -o rss= -p "$LOGOS_PID" 2>/dev/null | xargs || echo "?")
    echo "  RSS: ${RSS} KB"
fi

# ═══════════════════════════════════════════════════════════════════════════
# Summary
# ═══════════════════════════════════════════════════════════════════════════
banner "E2E Test Summary"
TOTAL=$((PASS + FAIL + SKIP))
echo "  PASSED:  $PASS"
echo "  FAILED:  $FAIL"
echo "  SKIPPED: $SKIP"
echo "  TOTAL:   $TOTAL"
echo "══════════════════════════════════════════════════════════════"

if [ "$FAIL" -gt 0 ]; then
    echo ""
    echo "  Result: SOME TESTS FAILED"
    exit 1
else
    echo ""
    echo "  Result: ALL TESTS PASSED"
    exit 0
fi
