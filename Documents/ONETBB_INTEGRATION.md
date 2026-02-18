# oneTBB Integration Guide

## Overview

LFList (Lock-Free List) uses Intel's oneTBB (oneAPI Threading Building Blocks) library for high-performance concurrent data structures. Specifically, it wraps `tbb::concurrent_queue` to provide a thread-safe, lock-free queue implementation.

## oneTBB Repository

- **URL**: https://github.com/uxlfoundation/oneTBB
- **License**: Apache 2.0
- **Platform Support**: Linux, macOS, Windows, and even WebAssembly
- **C++ Standard**: C++17 compatible

## Installation Options

### Option 1: System Installation (Recommended for Development)

Using Homebrew on macOS:
```bash
brew install tbb
```

Then link against system libraries:
```bash
g++ -std=gnu++17 ... -I/opt/homebrew/include -L/opt/homebrew/lib -ltbb
```

### Option 2: Local Build in .d Directory (Recommended for Project)

Clone and build oneTBB locally for the project:

```bash
# From project root
mkdir -p .d
cd .d
git clone https://github.com/uxlfoundation/oneTBB.git
cd oneTBB

# Build using CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=.. ..
cmake --build .
cmake --install .
```

This creates:
- `.d/oneTBB/include/` - Headers
- `.d/oneTBB/lib/` - Libraries (libtbb.dylib, libtbbmalloc.dylib)

### Option 3: Bif Integration (Future)

Future work: Create a bif dependency target that automates the download, build, and linking of oneTBB.

This would involve:
1. Adding oneTBB to the dependency specification in imap.yml or bif.js5
2. Creating build recipes for oneTBB compilation
3. Automatic inclusion of TBB headers and libraries in dependent targets

## Usage in CRUtil

### Current Files Using oneTBB

- `Source/include/lflist.h` - Lock-free list implementation
- `Test/test_lflist.cpp` - Comprehensive test suite

### Required Compiler Flags

```bash
-std=gnu++17                    # C++17 standard
-I <path-to-tbb>/include        # TBB headers
-L <path-to-tbb>/lib            # TBB libraries
-ltbb                           # Link against libtbb
```

### Example Compilation

If oneTBB is installed in `.d/oneTBB/`:

```bash
g++ -std=gnu++17 -D_GNU_SOURCE \
    -I Source/include \
    -I .d/oneTBB/include \
    myapp.cpp \
    -L .d/oneTBB/lib \
    -ltbb \
    -lpthread
```

## Runtime Configuration

### macOS Dynamic Library Path

If using a local build, you may need to set `DYLD_LIBRARY_PATH`:

```bash
export DYLD_LIBRARY_PATH=/Volumes/Development/DV/Live/CRUtil/.d/oneTBB/lib:$DYLD_LIBRARY_PATH
./test_runner
```

Or use install_name_tool to set the rpath:

```bash
install_name_tool -add_rpath @executable_path/../.d/oneTBB/lib test_runner
```

### Linux Dynamic Library Path

```bash
export LD_LIBRARY_PATH=/path/to/CRUtil/.d/oneTBB/lib:$LD_LIBRARY_PATH
./test_runner
```

## API Usage

LFList wraps `tbb::concurrent_queue` and provides:

```cpp
#include "lflist.h"

using namespace crutil;

LFList<int> queue;

// Producer thread
queue.push(42);                    // Thread-safe push

// Consumer thread
bool success;
int value = queue.remove(&success); // Thread-safe pop

// Wait for data
if (queue.waitFor(NS_IN_ONE_SEC)) { // Wait with timeout
    int value = queue.remove(&success);
}
```

## Key Features

- **Lock-free**: Uses atomic operations, no mutex contention
- **Thread-safe**: All operations are concurrent-safe
- **High-performance**: Optimized for producer-consumer patterns
- **Condition variable**: Built-in wait/signal mechanism
- **FIFO ordering**: First-in-first-out queue semantics

## Testing

Comprehensive test coverage includes:
- Basic operations (push, remove, clear)
- Concurrent operations (multiple producers/consumers)
- Edge cases (empty list, rapid cycles)
- Thread safety stress tests
- Producer-consumer patterns
- Timeout and signaling behavior

Run tests after installing oneTBB (see CLAUDE.md for commands).

## Performance Characteristics

- Push: O(1) amortized, lock-free
- Pop: O(1) amortized, lock-free
- Size: O(1) but may be approximate during concurrent access
- Empty: O(1) lock-free check
- WaitFor: Blocks efficiently using condition variables

## Future Work

1. Integrate oneTBB into bif build system
2. Add performance benchmarks comparing LFList vs CList
3. Consider templated allocators for custom memory management
4. Add support for bounded queues (size limits)
5. Profile memory usage patterns

## References

- [oneTBB Documentation](https://oneapi-src.github.io/oneTBB/)
- [oneTBB GitHub](https://github.com/uxlfoundation/oneTBB)
- [TBB concurrent_queue Reference](https://spec.oneapi.io/versions/latest/elements/oneTBB/source/containers/concurrent_queue_cls.html)
