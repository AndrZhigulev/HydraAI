# Building HydraAI C++23 Implementation

This document explains how to build and run the C++23 version of HydraAI with ImGui.

## Prerequisites

### Required Tools
- **CMake 3.20+** - Build system generator
- **C++23 Compatible Compiler**:
  - GCC 13+ (Linux)
  - Clang 16+ (macOS/Linux)
  - MSVC 19.34+ (Windows, Visual Studio 2022 17.4+)
- **Git** - For fetching dependencies

### Required Libraries

#### System Libraries (must be installed manually)

**Linux (Ubuntu/Debian)**:
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libsqlite3-dev \
    libgl1-mesa-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev
```

**Linux (Fedora/RHEL)**:
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    sqlite-devel \
    mesa-libGL-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel
```

**macOS**:
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake sqlite3
```

**Windows**:
- Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with C++ workload
- Install [CMake](https://cmake.org/download/)
- Install [vcpkg](https://vcpkg.io/):
  ```cmd
  git clone https://github.com/Microsoft/vcpkg.git
  cd vcpkg
  .\bootstrap-vcpkg.bat
  .\vcpkg integrate install
  ```
- Install SQLite3:
  ```cmd
  .\vcpkg install sqlite3:x64-windows
  ```

#### Automatically Fetched Dependencies

The following are automatically downloaded by CMake using FetchContent:
- **GLFW 3.4** - Window management and input
- **ImGui 1.90.4** - Immediate Mode GUI
- **nlohmann/json 3.11.3** - JSON parsing
- **cpp-httplib 0.15.3** - HTTP server/client

## Building

### 1. Clone the Repository

```bash
git clone <repository-url>
cd HydraAI
```

### 2. Create Build Directory

```bash
mkdir build
cd build
```

### 3. Configure CMake

**Linux/macOS**:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**Windows (Visual Studio)**:
```cmd
cmake .. -G "Visual Studio 17 2022" -A x64
```

**Windows (MinGW)**:
```cmd
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
```

### 4. Build

**Linux/macOS**:
```bash
make -j$(nproc)  # Build using all CPU cores
```

**Windows (Visual Studio)**:
```cmd
cmake --build . --config Release
```

**Windows (MinGW)**:
```cmd
mingw32-make -j8
```

### 5. Install (Optional)

```bash
sudo make install  # Linux/macOS
```

## Running

After building, you'll have three executables in the `build` directory:

### 1. Start the Coordinator

```bash
./hydra_coordinator
```

This starts the coordinator server with an ImGui dashboard showing:
- Active workers
- Task queue status
- Token distribution statistics
- Real-time training metrics

### 2. Start a Worker

```bash
./hydra_worker --user alice123 --server http://localhost:5000
```

The worker GUI shows:
- Connection status
- Current task progress
- Token balance
- Training visualizations (loss curves, accuracy)

### 3. Query the Model

```bash
./hydra_query --user alice123 --server http://localhost:5000
```

Interactive GUI for querying the trained model.

## Command Line Options

### hydra_coordinator
```
Usage: hydra_coordinator [OPTIONS]

Options:
  --port PORT          Server port (default: 5000)
  --host HOST          Server host (default: 0.0.0.0)
  --db PATH            Database path (default: hydra.db)
  --gui / --no-gui     Enable/disable GUI (default: gui)
  --help               Show this help message
```

### hydra_worker
```
Usage: hydra_worker [OPTIONS]

Options:
  --user USER_ID       User identifier (required)
  --server URL         Coordinator URL (default: http://localhost:5000)
  --threads NUM        Training threads (default: auto)
  --gui / --no-gui     Enable/disable GUI (default: gui)
  --help               Show this help message
```

### hydra_query
```
Usage: hydra_query [OPTIONS]

Options:
  --user USER_ID       User identifier (required)
  --server URL         Coordinator URL (default: http://localhost:5000)
  --interactive        Interactive mode
  --query "PROMPT"     Single query mode
  --help               Show this help message
```

## Troubleshooting

### CMake can't find SQLite3

**Linux**:
```bash
sudo apt-get install libsqlite3-dev  # Debian/Ubuntu
sudo dnf install sqlite-devel        # Fedora/RHEL
```

**macOS**:
```bash
brew install sqlite3
export PKG_CONFIG_PATH="/usr/local/opt/sqlite/lib/pkgconfig"
```

**Windows**:
```cmd
vcpkg install sqlite3:x64-windows
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
```

### C++23 Features Not Available

Make sure you have a recent compiler:
- GCC 13+ (check with `gcc --version`)
- Clang 16+ (check with `clang --version`)
- MSVC 19.34+ (Visual Studio 2022 17.4+)

**Update GCC on Ubuntu**:
```bash
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-13 g++-13
export CXX=g++-13
```

### OpenGL/Graphics Issues

**Linux**: Make sure you have graphics drivers installed
```bash
# NVIDIA
sudo apt-get install nvidia-driver-XXX

# AMD
sudo apt-get install mesa-vulkan-drivers

# Intel
sudo apt-get install intel-media-va-driver
```

**Windows**: Update your graphics drivers from manufacturer website

### Build is Very Slow

First build downloads dependencies. Subsequent builds are fast.

To speed up:
```bash
make -j$(nproc)  # Use all CPU cores
```

## Development

### Enable Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Enable Compiler Warnings

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"
```

### IDE Integration

**CLion**: Open the project root directory

**Visual Studio Code**: Install CMake Tools extension, open project root

**Visual Studio 2022**: Open CMakeLists.txt as a project

## Performance Optimization

### Release Build with Optimizations

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_CXX_FLAGS="-O3 -march=native -flto"
```

Flags explained:
- `-O3`: Maximum optimization
- `-march=native`: Optimize for your CPU
- `-flto`: Link-time optimization

### Profile-Guided Optimization (PGO)

```bash
# 1. Build with profiling
cmake .. -DCMAKE_CXX_FLAGS="-fprofile-generate"
make

# 2. Run typical workload
./hydra_worker --user test

# 3. Rebuild with profile data
cmake .. -DCMAKE_CXX_FLAGS="-fprofile-use"
make
```

## Cross-Compilation

### Linux to Windows (MinGW)

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake
make
```

### Building for Raspberry Pi

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/rpi-toolchain.cmake
make
```

## Next Steps

- See [ARCHITECTURE_CPP.md](ARCHITECTURE_CPP.md) for system design
- See [API.md](API.md) for HTTP API documentation
- See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines

## Support

- GitHub Issues: <repository-url>/issues
- Documentation: <repository-url>/docs
- Discord: [Link to Discord]
