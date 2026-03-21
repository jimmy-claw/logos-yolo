# Contributing to logos-yolo-module

## Development setup

### Prerequisites

- CMake 3.16+
- C++17 compiler (GCC 9+ or Clang 10+)
- Qt6 (Core, Qml, Test; add Widgets, Quick, QuickWidgets for UI builds)
- [logos-pipe](https://github.com/jimmy-claw/logos-pipe) (branch: `feature/federated-channel`)

**Ubuntu/Debian:**

```bash
sudo apt install cmake qt6-base-dev qt6-declarative-dev qt6-remoteobjects-dev g++
```

### Clone and build

```bash
git clone https://github.com/jimmy-claw/logos-yolo.git
git clone -b feature/federated-channel https://github.com/jimmy-claw/logos-pipe.git
cd logos-yolo

cmake -B build -DBUILD_TESTS=ON -DLOGOS_PIPE_ROOT=../logos-pipe
cmake --build build -j$(nproc)
```

## Running tests

The test suite uses Qt6 Test and runs via CTest. All tests use mock clients — no blockchain required.

```bash
ctest --output-on-failure --test-dir build
```

Individual test executables are in `build/`:

| Test | What it covers |
|------|---------------|
| `test_yolo_board` | Board CRUD, JSON serialization, discovery |
| `test_yolo_events` | Signal chain verification |
| `test_yolo_federation` | Multi-admin federation, merged history |
| `test_yolo_event_lifecycle` | Full 6-event sequence (post.created → post.published) |
| `test_integration` | End-to-end: create board → post → read → verify |

Integration test script:

```bash
bash tests/run_integration.sh
```

### Nix builds

```bash
nix build .              # headless module
nix build .#ui-plugin    # UI plugin
nix build .#lgx          # .lgx bundle
```

## Code style

There is no automated formatter yet. Follow these conventions:

- **C++ standard:** C++17
- **Header guards:** `#pragma once`
- **Classes:** PascalCase (`YoloBoard`, `YoloPlugin`)
- **Methods:** camelCase (`createPost`, `getPosts`)
- **Private members:** `m_` prefix (`m_boards`, `m_pipe`)
- **Qt patterns:** Use `Q_OBJECT`, `Q_PROPERTY`, `Q_INVOKABLE`, signals/slots
- **Include order:** Qt headers first, then project headers

## Adding a feature

1. **Create a branch** off `master`.
2. **Write tests first** in `tests/` using the Qt6 Test framework. Reference `tests/test_yolo_board.cpp` for examples.
3. **Implement** in `src/`. Key integration points:
   - `FederatedChannel` — publish/subscribe over logos-pipe (`logos-pipe/src/federated_channel.h`)
   - `ContentStore` — content-addressed blob storage (`logos-pipe/src/content_store.h`)
   - `ChannelIndexer` — channel discovery and indexing (`logos-pipe/src/channel_indexer.h`)
4. **Verify locally:**
   ```bash
   cmake -B build -DBUILD_TESTS=ON -DLOGOS_PIPE_ROOT=../logos-pipe
   cmake --build build -j$(nproc)
   ctest --output-on-failure --test-dir build
   nix build .
   ```
5. **Open a PR** against `master` using the PR template.
