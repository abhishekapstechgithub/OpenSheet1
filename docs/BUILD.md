# Building OpenSheet

Detailed build instructions for all supported platforms.

---

## Quick Start

```bash
git clone https://github.com/opensheet/opensheet.git
cd opensheet
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/opensheet
```

---

## Prerequisites

### All Platforms

| Tool | Min Version | Install |
|------|-------------|---------|
| CMake | 3.20 | cmake.org / package manager |
| Qt6 | 6.4 | qt.io/download or package manager |
| SQLite3 | 3.38 | system package |
| Ninja | 1.11 | optional but faster than make |
| Python 3 | 3.8 | optional (plugin support) |

### Linux (Ubuntu / Debian)

```bash
# Qt6 from Ubuntu 22.04 repos
sudo apt update
sudo apt install \
    qt6-base-dev \
    qt6-charts-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    libqt6svg6-dev \
    libqt6sql6-sqlite \
    libsqlite3-dev \
    cmake \
    ninja-build \
    gcc-12 g++-12 \
    libgl1-mesa-dev \
    libxkbcommon-dev \
    libvulkan-dev \
    python3-dev

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100
```

### Linux (Fedora / RHEL)

```bash
sudo dnf install \
    qt6-qtbase-devel \
    qt6-qtcharts-devel \
    qt6-qtsvg-devel \
    sqlite-devel \
    cmake \
    ninja-build \
    gcc-c++ \
    python3-devel
```

### Linux (Arch)

```bash
sudo pacman -S qt6-base qt6-charts qt6-svg sqlite cmake ninja python
```

### macOS

```bash
# Install Homebrew if needed: https://brew.sh
brew install cmake ninja sqlite3 python3

# Install Qt6 via Qt Online Installer or Homebrew
brew install qt@6
echo 'export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Windows (MSVC 2022)

1. Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with "Desktop development with C++" workload
2. Install [Qt6](https://www.qt.io/download) — select MSVC 2019 64-bit component
3. Install [CMake](https://cmake.org/download/)
4. Install SQLite3 via [vcpkg](https://vcpkg.io/):
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   C:\vcpkg\vcpkg integrate install
   C:\vcpkg\vcpkg install sqlite3:x64-windows
   ```

---

## Build Configurations

### Debug (for development)

```bash
cmake -B build-debug -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DOPENSHEET_BUILD_TESTS=ON \
    -DOPENSHEET_WARNINGS_AS_ERRORS=OFF
cmake --build build-debug --parallel
```

### Release (for distribution)

```bash
cmake -B build-release -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPENSHEET_BUILD_TESTS=OFF
cmake --build build-release --parallel
```

### With AddressSanitizer

```bash
cmake -B build-asan -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DOPENSHEET_SANITIZE=address \
    -DOPENSHEET_BUILD_TESTS=ON
cmake --build build-asan --parallel
```

### With Python Plugin Support

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DOPENSHEET_PYTHON_SUPPORT=ON
cmake --build build --parallel
```

### CMake Options Reference

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Debug / Release / RelWithDebInfo |
| `OPENSHEET_BUILD_TESTS` | `ON` | Build Qt Test unit tests |
| `OPENSHEET_PYTHON_SUPPORT` | auto | Enable Python plugin loader |
| `OPENSHEET_SANITIZE` | `none` | address / undefined / thread / none |
| `OPENSHEET_WARNINGS_AS_ERRORS` | `OFF` | `-Werror` / `/WX` |

---

## Running Tests

```bash
cd build-debug

# Run all tests
ctest --output-on-failure --parallel $(nproc)

# Run only engine tests
ctest -L engine --output-on-failure

# Run only file I/O tests
ctest -L fileio --output-on-failure

# Run a specific test binary directly (more verbose)
./tests/test_formula_parser
./tests/test_cell

# With valgrind (Linux)
valgrind --error-exitcode=1 --leak-check=full ./tests/test_engine
```

---

## Windows (Visual Studio GUI)

1. Open Visual Studio 2022
2. `File → Open → CMake...`
3. Select `CMakeLists.txt` in the project root
4. Visual Studio auto-configures the project
5. Set Qt6 path: `Project → CMake Settings → CMakeCommandArgs`:
   ```
   -DCMAKE_PREFIX_PATH=C:\Qt\6.6.3\msvc2019_64
   ```
6. Build: `Build → Build All` or `Ctrl+Shift+B`

---

## macOS Bundle

```bash
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build build --parallel

# Bundle Qt into .app
$Qt6_DIR/bin/macdeployqt build/bin/opensheet.app

# Create DMG
cd installer/macos
./build_dmg.sh ../../build ../../dist
```

---

## Linux AppImage

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

cd installer/linux
./build_appimage.sh ../../build ../../dist
# Output: dist/OpenSheet-1.0.0-x86_64.AppImage
```

---

## Windows Installer (.exe)

1. Build Release for Windows (see Windows section above)
2. Run `windeployqt` on the exe:
   ```powershell
   & "C:\Qt\6.6.3\msvc2019_64\bin\windeployqt.exe" --release build\bin\Release\opensheet.exe
   ```
3. Open `installer/windows/opensheet.iss` in [Inno Setup](https://jrsoftware.org/isinfo.php)
4. Build → Compile
5. Output: `installer/windows/output/OpenSheet-Setup-1.0.0-x64.exe`

---

## IDE Setup

### VS Code

Install extensions:
- `CMake Tools` (Microsoft)
- `C/C++` (Microsoft)
- `clangd` (optional, for better IntelliSense)

`.vscode/settings.json`:
```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build-debug",
  "cmake.configureSettings": {
    "CMAKE_BUILD_TYPE": "Debug",
    "OPENSHEET_BUILD_TESTS": "ON"
  }
}
```

### CLion

- Open `CMakeLists.txt` directly
- CLion auto-detects profiles
- Set Qt6 path in `CMake Options` if not found automatically

### Qt Creator

- `File → Open File or Project → CMakeLists.txt`
- Configure kit with Qt 6.x
- Build with `Ctrl+B`
