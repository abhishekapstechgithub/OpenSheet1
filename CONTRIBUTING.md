# Contributing to OpenSheet

Thank you for your interest in contributing! This guide covers everything you need to get started.

---

## Table of Contents
1. [Development Setup](#development-setup)
2. [Branch Strategy](#branch-strategy)
3. [Code Style](#code-style)
4. [Submitting a PR](#submitting-a-pr)
5. [Running Tests](#running-tests)
6. [Writing Tests](#writing-tests)
7. [Adding a Formula](#adding-a-formula)
8. [Adding a File Format](#adding-a-file-format)

---

## Development Setup

### Prerequisites

| Tool       | Minimum Version |
|------------|-----------------|
| CMake      | 3.20            |
| Qt6        | 6.4             |
| GCC/Clang  | C++20 support   |
| MSVC       | 2022 (Windows)  |
| Python 3   | 3.8 (optional)  |
| SQLite3    | 3.38            |

### Clone and Build

```bash
git clone https://github.com/opensheet/opensheet.git
cd opensheet
git submodule update --init --recursive  # if any

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -DOPENSHEET_BUILD_TESTS=ON
cmake --build . --parallel
ctest --output-on-failure
```

### IDE Setup

**VS Code**: Install the *CMake Tools* and *clangd* extensions.
```json
// .vscode/settings.json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureArgs": ["-DOPENSHEET_BUILD_TESTS=ON"],
  "clangd.arguments": ["--compile-commands-dir=${workspaceFolder}/build"]
}
```

**Qt Creator**: Open `CMakeLists.txt` directly. Creator detects Qt automatically.

**CLion**: Open the root folder. CLion reads `CMakeLists.txt` and configures automatically.

---

## Branch Strategy

| Branch        | Purpose                            |
|---------------|------------------------------------|
| `main`        | Stable release branch              |
| `develop`     | Integration branch for features    |
| `feature/xyz` | Individual feature development     |
| `bugfix/xyz`  | Bug fixes targeting `develop`      |
| `hotfix/xyz`  | Critical fixes targeting `main`    |
| `release/x.y` | Release preparation                |

All PRs target `develop`. We merge `develop` → `main` for releases.

---

## Code Style

We use `clang-format` with the config in `.clang-format`.

Before committing, format all modified files:
```bash
find office plugins main.cpp -name "*.cpp" -o -name "*.h" | \
  xargs clang-format -i --style=file
```

Or install a pre-commit hook:
```bash
cp scripts/pre-commit .git/hooks/pre-commit
chmod +x .git/hooks/pre-commit
```

### Key conventions

- **Namespace**: all engine/core/UI code lives in `namespace OpenSheet`
- **Naming**: `CamelCase` for classes, `camelCase` for methods/variables, `m_` prefix for members, `k` prefix for constants
- **Headers**: use `#pragma once`, forward-declare where possible
- **Qt**: prefer `QObject`-derived classes for anything that needs signals/slots; use `nullptr` not `NULL`; use `qDebug` / `qWarning` for logging
- **Memory**: use Qt parent-child ownership; avoid raw `new` in business logic

---

## Submitting a PR

1. Fork the repo and create a branch: `git checkout -b feature/my-feature develop`
2. Make your changes and commit with clear messages: `git commit -m "feat(engine): add XLOOKUP formula"`
3. Ensure tests pass: `ctest --output-on-failure`
4. Ensure formatting is clean: `clang-format --dry-run --Werror`
5. Push and open a PR targeting `develop`
6. Fill in the PR template completely

---

## Running Tests

```bash
cd build
ctest --output-on-failure          # all tests
ctest -R test_engine               # filter by name
ctest --verbose                    # verbose output
./tests/test_cell                  # run a single test binary directly
```

---

## Writing Tests

Tests use Qt Test (`QTest`). Add a new test class in `tests/`:

```cpp
// tests/test_myfeature.cpp
#include <QtTest>
#include "myfeature.h"

class TestMyFeature : public QObject {
    Q_OBJECT
private slots:
    void testSomething() {
        QCOMPARE(myFunction(2, 3), 5);
    }
};

QTEST_MAIN(TestMyFeature)
#include "test_myfeature.moc"
```

Register it in `tests/CMakeLists.txt`:
```cmake
add_executable(test_myfeature test_myfeature.cpp)
target_link_libraries(test_myfeature PRIVATE os_engine Qt6::Test)
add_test(NAME MyFeatureTests COMMAND test_myfeature)
```

---

## Adding a Formula

1. Add the declaration in `office/engine/formula_parser.h`:
```cpp
static QVariant fnMyFormula(const QVector<QVariant>&, ParseContext&);
```

2. Implement in `office/engine/formula_parser.cpp`:
```cpp
QVariant FormulaParser::fnMyFormula(const QVector<QVariant>& args, ParseContext&) {
    if (args.size() < 1) return QVariant();
    // ... implementation
    return result;
}
```

3. Register in `FormulaParser::registerBuiltins()`:
```cpp
m_functions["MYFORMULA"] = fnMyFormula;
```

4. Add a test in `tests/test_formula_parser.cpp`:
```cpp
void testFn_MYFORMULA() {
    FormulaParser p(wb);
    ParseContext ctx{sheet, wb};
    QCOMPARE(p.evaluate("=MYFORMULA(A1)", ctx).toDouble(), expectedValue);
}
```

---

## Adding a File Format

1. Create `office/core/file_system/myformat_reader.h/cpp`
2. Implement read/write functions returning `Workbook*` / `bool`
3. Add a branch in `FileManager::detectFormat()` and `FileManager::open()` / `FileManager::save()`
4. Add to `office/core/CMakeLists.txt`
5. Add filter string in `MainWindow::onOpen()` / `onSaveAs()`
6. Write tests in `tests/test_myformat.cpp`
