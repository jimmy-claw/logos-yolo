#!/usr/bin/env bash
# demo-post.sh — Exercises the full YOLO post lifecycle via the test suite
#
# This script builds the test binaries and runs them with verbose output,
# narrating each step of the board creation, posting, and reading flow.
#
# Prerequisites: cmake, Qt6, logos-pipe source tree
# Usage: ./examples/demo-post.sh [path-to-logos-pipe]

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
NC='\033[0m' # No Color

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
banner "YOLO Community Board Demo"

step "Building test suite..."
info "Project: $PROJECT_DIR"
info "logos-pipe: $LOGOS_PIPE_ROOT"

cmake -B "$BUILD_DIR" -S "$PROJECT_DIR" \
    -DBUILD_TESTS=ON \
    -DLOGOS_PIPE_ROOT="$LOGOS_PIPE_ROOT" \
    -DCMAKE_BUILD_TYPE=Debug \
    > /dev/null 2>&1

cmake --build "$BUILD_DIR" -j"$(nproc)" > /dev/null 2>&1

step "Build complete."
echo ""

# ── Demo 1: Board Operations ─────────────────────────────────────────────────
banner "1. Board Operations (create, post, read)"

step "Creating board, posting messages, verifying event pipeline..."
info "This runs test_yolo_board which exercises:"
info "  - Board creation with prefix YOLO:<name>"
info "  - Channel ID derivation (SHA-256)"
info "  - Post serialization round-trip (JSON)"
info "  - Post creation with signal chain verification"
info "  - Large content offload to ContentStore (>1KB)"
info "  - Post reading with newest-first ordering"
info "  - Board discovery via prefix scan"
echo ""

# Expected output:
# PASS   : TestYoloBoard::testBoardCreation()
# PASS   : TestYoloBoard::testPostSerialization()
# PASS   : TestYoloBoard::testCreatePost_signalChain()
# PASS   : TestYoloBoard::testGetPosts_deserializationAndOrdering()
# PASS   : TestYoloBoard::testDiscoverBoards()
# ...
"$BUILD_DIR/test_yolo_board" -v2 2>&1 | while IFS= read -r line; do
    if [[ "$line" == *"PASS"* ]]; then
        echo -e "  ${GREEN}✓${NC} $line"
    elif [[ "$line" == *"FAIL"* ]]; then
        echo -e "  \033[0;31m✗${NC} $line"
    elif [[ "$line" == *"QDEBUG"* ]]; then
        # Strip QDEBUG prefix for cleaner output
        msg="${line#*- }"
        echo -e "  ${CYAN}→${NC} $msg"
    fi
done

echo ""

# ── Demo 2: Event Pipeline ───────────────────────────────────────────────────
banner "2. Event Pipeline (post lifecycle signals)"

step "Verifying all 6 lifecycle events fire in correct order..."
info "Small post (<=1KB): post.created → post.inscribing → post.inscribed → post.published"
info "Large post (>1KB):  post.created → post.uploading → post.uploaded → post.inscribing → post.inscribed → post.published"
echo ""

# Expected output:
# PASS   : TestYoloEvents::testSmallPost_fullSignalSequence()
# PASS   : TestYoloEvents::testLargePost_fullSignalSequence()
# PASS   : TestYoloEvents::testErrorCondition_channelNotAvailable()
# ...
"$BUILD_DIR/test_yolo_events" -v2 2>&1 | while IFS= read -r line; do
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

# ── Demo 3: Full Integration ─────────────────────────────────────────────────
banner "3. Full Integration (end-to-end flow)"

step "Running full pipeline: board → post → events → verify..."
info "This exercises the complete flow as Basecamp would use it:"
info "  1. Create board (inscribes board_meta)"
info "  2. Create post (inscribes post JSON)"
info "  3. Verify 4 events fired in order"
info "  4. Board discovery returns channel list"
info "  5. Large content auto-offloads to ContentStore"
info "  6. Yolo QML API wraps everything correctly"
echo ""

# Expected output:
# PASS   : TestIntegration::testFullFlow_createBoardPostAndVerify()
# PASS   : TestIntegration::testDiscoverBoards()
# PASS   : TestIntegration::testMultipleBoards()
# PASS   : TestIntegration::testLargeContentStorageOffload()
# PASS   : TestIntegration::testYoloQmlApiFlow()
"$BUILD_DIR/test_integration" -v2 2>&1 | while IFS= read -r line; do
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
banner "Demo Complete"

step "What just happened:"
info "1. Created boards with federated channels (YOLO:<name> prefix)"
info "2. Posted messages — small ones inscribed directly, large ones offloaded to storage"
info "3. Read back posts in reverse chronological order (newest first)"
info "4. Verified the event pipeline fires all lifecycle events"
info "5. Confirmed board discovery finds all YOLO channels"
info "6. Validated the QML API layer works end-to-end"
echo ""
step "Next: try loading the UI plugin in Basecamp (see README.md)"
echo ""
