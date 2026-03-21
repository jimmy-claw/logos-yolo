#!/usr/bin/env bash
# multi-admin-demo.sh — Demonstrates federated multi-admin posting
#
# Two admins post to the same board through separate channels.
# A reader sees a merged, chronological feed from both writers.
# This is the core federation model: no single writer controls the board.
#
# Prerequisites: cmake, Qt6, logos-pipe source tree
# Usage: ./examples/multi-admin-demo.sh [path-to-logos-pipe]

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
LOGOS_PIPE_ROOT="${1:-${LOGOS_PIPE_ROOT:-$PROJECT_DIR/../logos-pipe}}"
BUILD_DIR="$PROJECT_DIR/build-demo"

# ── Colors ────────────────────────────────────────────────────────────────────
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m'

banner() { echo -e "\n${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"; echo -e "${CYAN}  $1${NC}"; echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}\n"; }
step()   { echo -e "${GREEN}▸${NC} $1"; }
info()   { echo -e "${YELLOW}  ℹ${NC} $1"; }

# ── Validate logos-pipe ───────────────────────────────────────────────────────
if [ ! -d "$LOGOS_PIPE_ROOT/src" ]; then
    echo "ERROR: logos-pipe not found at $LOGOS_PIPE_ROOT"
    echo "Usage: $0 [path-to-logos-pipe]"
    echo "  or set LOGOS_PIPE_ROOT environment variable"
    exit 1
fi

# ── Build ─────────────────────────────────────────────────────────────────────
banner "YOLO Federation Demo: Multi-Admin Posting"

step "Building test suite..."
cmake -B "$BUILD_DIR" -S "$PROJECT_DIR" \
    -DBUILD_TESTS=ON \
    -DLOGOS_PIPE_ROOT="$LOGOS_PIPE_ROOT" \
    -DCMAKE_BUILD_TYPE=Debug \
    > /dev/null 2>&1

cmake --build "$BUILD_DIR" -j"$(nproc)" > /dev/null 2>&1

step "Build complete."
echo ""

# ── Federation Model Explanation ──────────────────────────────────────────────
banner "How Federation Works"

echo -e "${MAGENTA}  In YOLO, a board is a FederatedChannel — a collection of${NC}"
echo -e "${MAGENTA}  per-admin blockchain channels merged into one logical feed.${NC}"
echo ""
echo -e "  Board: ${CYAN}YOLO:community${NC}"
echo -e "  ├── Admin 1 (alice): writes to ${CYAN}SHA-256(\"λYOLO:community:<alice_pubkey>\")${NC}"
echo -e "  ├── Admin 2 (bob):   writes to ${CYAN}SHA-256(\"λYOLO:community:<bob_pubkey>\")${NC}"
echo -e "  └── Reader:          sees merged feed from both channels"
echo ""
info "No single admin can censor the other — each writes independently."
info "Readers merge all admin channels chronologically."
echo ""

# ── Demo: Two Admins, One Board ───────────────────────────────────────────────
banner "Test: Two Admins Post to Same Board"

step "Running federation test suite..."
info "test_yolo_federation exercises:"
info "  - Two admins posting to the same board prefix"
info "  - Reader sees merged chronological history from both"
info "  - Admin add/remove signals"
info "  - Follow/unfollow state management"
echo ""

# Expected output:
# PASS   : TestYoloFederation::testTwoAdminsPostToSameBoard()
# PASS   : TestYoloFederation::testReaderSeesMergedChronological()
# PASS   : TestYoloFederation::testAdminAddedSignal()
# PASS   : TestYoloFederation::testAdminRemovedSignal()
# PASS   : TestYoloFederation::testFederatedChannelAdminManagement()
# PASS   : TestYoloFederation::testFederatedChannelFollowState()
"$BUILD_DIR/test_yolo_federation" -v2 2>&1 | while IFS= read -r line; do
    if [[ "$line" == *"PASS"* ]]; then
        echo -e "  ${GREEN}✓${NC} $line"
    elif [[ "$line" == *"FAIL"* ]]; then
        echo -e "  \033[0;31m✗${NC} $line"
    elif [[ "$line" == *"QDEBUG"* ]]; then
        msg="${line#*- }"
        echo -e "  ${CYAN}→${NC} $msg"
    fi
done

echo ""

# ── Event Lifecycle with Federation ───────────────────────────────────────────
banner "Test: Event Lifecycle Across Full Pipeline"

step "Running event lifecycle tests..."
info "Verifying events propagate correctly through the full chain:"
info "  YoloBoard → Yolo → YoloPlugin (signal forwarding)"
echo ""

"$BUILD_DIR/test_yolo_event_lifecycle" -v2 2>&1 | while IFS= read -r line; do
    if [[ "$line" == *"PASS"* ]]; then
        echo -e "  ${GREEN}✓${NC} $line"
    elif [[ "$line" == *"FAIL"* ]]; then
        echo -e "  \033[0;31m✗${NC} $line"
    elif [[ "$line" == *"QDEBUG"* ]]; then
        msg="${line#*- }"
        echo -e "  ${CYAN}→${NC} $msg"
    fi
done

echo ""

# ── Summary ───────────────────────────────────────────────────────────────────
banner "Federation Demo Complete"

step "What this demonstrated:"
info "1. Two independent admins posted to the same board"
info "2. Each admin wrote to their own derived channel (SHA-256 of prefix + pubkey)"
info "3. A reader saw a merged, chronological feed from both admins"
info "4. Admin management (add/remove) is tracked with signals"
info "5. Events propagate through the full signal chain: Board → Yolo → Plugin"
echo ""
echo -e "${MAGENTA}  This is censorship resistance: no single party controls the board.${NC}"
echo -e "${MAGENTA}  Remove one admin, and the rest keep posting independently.${NC}"
echo ""
