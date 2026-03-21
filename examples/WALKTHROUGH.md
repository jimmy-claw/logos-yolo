# YOLO Workshop Walkthrough

Step-by-step guide for the ETHCluj workshop. By the end, you'll have built the YOLO module from source, run the test suite, seen the event pipeline in action, and (optionally) loaded the UI in Basecamp.

## Prerequisites

- Linux or macOS
- CMake 3.16+
- Qt6 development packages (Core, Qml, Test)
- A C++17 compiler (GCC 9+ or Clang 10+)
- Git

**On Ubuntu/Debian:**
```bash
sudo apt install cmake qt6-base-dev qt6-declarative-dev qt6-remoteobjects-dev g++
```

**On Nix (crib):**
```bash
# Qt6 and CMake are available in the crib environment — no extra setup needed
```

## Step 1: Clone the Repositories

```bash
git clone https://github.com/jimmy-claw/logos-yolo.git
git clone -b feature/federated-channel https://github.com/jimmy-claw/logos-pipe.git
cd logos-yolo
```

You need both repos because YOLO's board logic (`YoloBoard`) depends on logos-pipe's `FederatedChannel`, `ContentStore`, `ChannelIndexer`, and `ChannelSync`.

## Step 2: Build the Test Suite

```bash
cmake -B build -DBUILD_TESTS=ON -DLOGOS_PIPE_ROOT=../logos-pipe
cmake --build build -j$(nproc)
```

This compiles 5 test executables:

| Binary | What it tests |
|--------|---------------|
| `test_yolo_board` | Board CRUD, post serialization, discovery |
| `test_yolo_events` | Event signal chain for post lifecycle |
| `test_yolo_federation` | Multi-admin federation, merged history |
| `test_yolo_event_lifecycle` | Full 6-event sequence end-to-end |
| `test_integration` | Complete flow: board → post → read → verify |

## Step 3: Run All Tests

```bash
ctest --output-on-failure --test-dir build
```

Expected output:
```
Test project /path/to/logos-yolo/build
    Start 1: test_yolo_board
1/5 Test #1: test_yolo_board ................   Passed    0.05 sec
    Start 2: test_yolo_events
2/5 Test #2: test_yolo_events ...............   Passed    0.04 sec
    Start 3: test_yolo_federation
3/5 Test #3: test_yolo_federation ...........   Passed    0.04 sec
    Start 4: test_yolo_event_lifecycle
4/5 Test #4: test_yolo_event_lifecycle ......   Passed    0.04 sec
    Start 5: test_integration
5/5 Test #5: test_integration ...............   Passed    0.04 sec

100% tests passed, 0 tests failed out of 5
```

All tests use mock clients (no real blockchain needed). The mocks simulate inscription IDs, channel history, and storage uploads.

## Step 4: Run the Demo Scripts

### Full post lifecycle demo

```bash
./examples/demo-post.sh ../logos-pipe
```

This builds and runs three test suites with narrated output showing:
1. Board creation with `YOLO:<name>` prefix
2. Post creation with event signals firing
3. Post reading with newest-first ordering
4. Board discovery via prefix scan
5. Full integration pipeline

### Federation demo

```bash
./examples/multi-admin-demo.sh ../logos-pipe
```

This demonstrates the core censorship-resistance model:
1. Two admins (different pubkeys) post to the same board
2. Each admin writes to their own derived channel: `SHA-256("λYOLO:<name>:<pubkey>")`
3. A reader sees a merged chronological feed from both admins
4. Admin add/remove operations are tracked with signals

## Step 5: Understand the Event Pipeline

Every post goes through a lifecycle that emits observable events. These events power the Event Log in the QML UI and are available to any consumer via the `eventResponse` signal.

### Small post (<=1KB) — 4 events

```
post.created    [info]     Post object constructed
post.inscribing [info]     Writing to blockchain channel
post.inscribed  [success]  Inscription confirmed with ID
post.published  [success]  Full pipeline complete
```

### Large post (>1KB) — 6 events

```
post.created    [info]     Post object constructed
post.uploading  [info]     Content exceeds 1KB, uploading to storage
post.uploaded   [success]  ContentStore returned CID
post.inscribing [info]     Writing inscription (with CID reference)
post.inscribed  [success]  Inscription confirmed with ID
post.published  [success]  Full pipeline complete
```

The signal cascade flows through the module layers:

```
YoloBoard::eventResponse(name, [type, message])
    ↓ connected
Yolo::newEvent(name, type, message)        → QML EventLog
Yolo::eventResponse(name, [type, message]) → forwarded
    ↓ connected
YoloPlugin::eventResponse(name, [type, message])  → logoscore
```

## Step 6: Load in Basecamp (Optional)

This requires the full Logos SDK (logos-cpp-sdk, logos-liblogos) which are available on crib.

### Using Make targets

```bash
# Set up merged SDK dirs from Nix store
make setup-nix-merged

# Build and install everything
make install-all

# Launch logos-app
cd ~/logos-workspace && nix run .#logos-app-poc
```

### Manual installation

```bash
# Build UI plugin
cmake -B build-ui -DBUILD_UI_PLUGIN=ON \
  -DLOGOS_PIPE_ROOT=../logos-pipe \
  -DLOGOS_CPP_SDK_ROOT=/path/to/logos-cpp-sdk \
  -DLOGOS_LIBLOGOS_ROOT=/path/to/logos-liblogos
cmake --build build-ui --target yolo_ui -j$(nproc)

# Install UI plugin
mkdir -p ~/.local/share/Logos/LogosAppNix/plugins/yolo_ui
cp build-ui/libyolo_ui.so ~/.local/share/Logos/LogosAppNix/plugins/yolo_ui/yolo_ui.so

# Build headless module
cmake -B build-module -DBUILD_MODULE=ON \
  -DLOGOS_CPP_SDK_ROOT=/path/to/logos-cpp-sdk \
  -DLOGOS_LIBLOGOS_ROOT=/path/to/logos-liblogos
cmake --build build-module --target yolo_plugin -j$(nproc)

# Install headless module
mkdir -p ~/.local/share/Logos/LogosAppNix/modules/yolo
cp build-module/yolo_plugin.so ~/.local/share/Logos/LogosAppNix/modules/yolo/
cp metadata.json ~/.local/share/Logos/LogosAppNix/modules/yolo/manifest.json
```

### What the UI looks like

The QML interface provides:

- **Board List** — discover existing boards or create new ones
- **Board View** — see all posts from all admins, newest first
- **Create Post** — compose a post with title and content
- **Event Log** — real-time stream of lifecycle events (toggle with "Log" button)

The UI is a `QQuickWidget` loaded as an `IComponent` plugin in logos-app. The `Yolo` backend is exposed as a QML context property named `yolo`.

## Key Concepts

### Channel ID Derivation

Every admin gets a unique blockchain channel derived deterministically:

```
channelId = SHA-256("λ" + prefix + ":" + pubkey)
```

For example, if the board is `YOLO:community` and the admin's pubkey is `abc123`:

```
channelId = SHA-256("λYOLO:community:abc123")  →  "7f3a..."
```

This means:
- Given a prefix and pubkey, anyone can compute the channel ID
- No registration or coordination needed
- Discovery works by scanning for all channels with a given prefix

### Content Offloading

Posts with content > 1KB are automatically offloaded:

1. Content is stored in `ContentStore` → returns a CID (content-addressed ID)
2. The post inscription contains `"cid:<CID>"` instead of the full content
3. Readers fetch the content by CID when needed

This keeps on-chain inscriptions small while supporting arbitrarily large posts.

### Federation Model

A YOLO board is not a single channel — it's a `FederatedChannel` that merges multiple admin channels:

```
FederatedChannel("YOLO:community")
├── Alice's channel: SHA-256("λYOLO:community:<alice>")
├── Bob's channel:   SHA-256("λYOLO:community:<bob>")
└── Reader: merged history from all channels, sorted by slot
```

No single admin can censor another. Each writes independently. Readers merge.

## Troubleshooting

### `Qt6 not found`

Make sure Qt6 development packages are installed. On Ubuntu:
```bash
sudo apt install qt6-base-dev qt6-declarative-dev
```

On Nix, enter the dev shell or crib environment that provides Qt6.

### `LOGOS_PIPE_ROOT not set`

The test build requires logos-pipe. Pass it explicitly:
```bash
cmake -B build -DBUILD_TESTS=ON -DLOGOS_PIPE_ROOT=/path/to/logos-pipe
```

### Tests pass but UI won't build

The UI plugin requires additional Qt6 modules (Widgets, Quick, QuickWidgets) and the Logos SDK. Tests only need Qt6 Core and Test.

### `logos_core.so not found`

The headless module and UI plugin (with Logos Core integration) need `logos-liblogos`. This is only available on crib or via Nix. For testing without Logos Core, use `BUILD_TESTS=ON` which doesn't require the SDK.
