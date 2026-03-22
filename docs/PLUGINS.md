# OpenSheet Plugin Development Guide

---

## Overview

OpenSheet supports two types of plugins:

| Type | Language | Use For |
|------|----------|---------|
| **C++ Plugin** | C++20 + Qt6 | High-performance formulas, deep UI integration |
| **Python Plugin** | Python 3.8+ | Quick scripting, data tools, automation |

Both types are auto-discovered at startup from the `addons/plugins/` directory.

---

## C++ Plugin

### File Structure

```
addons/plugins/my_plugin/
├── CMakeLists.txt      ← build as shared library
├── my_plugin.h         ← implements IPlugin
├── my_plugin.cpp
└── my_plugin.json      ← manifest (optional)
```

### Minimal Plugin

```cpp
// my_plugin.h
#pragma once
#include <QObject>
#include "../../plugins/plugin_manager.h"

class MyPlugin : public QObject, public OpenSheet::IPlugin {
    Q_OBJECT
    Q_INTERFACES(OpenSheet::IPlugin)
    Q_PLUGIN_METADATA(IID "io.opensheet.IPlugin/1.0")
public:
    QString name()        const override { return "My Plugin"; }
    QString version()     const override { return "1.0.0"; }
    QString description() const override { return "Does something great."; }
    QString author()      const override { return "Your Name"; }

    bool initialize(OpenSheet::PluginContext *ctx) override {
        ctx->registerFormula("DOUBLE", [](const QVector<QVariant> &args,
                                          OpenSheet::ParseContext &) -> QVariant {
            return args.isEmpty() ? 0.0 : args[0].toDouble() * 2.0;
        });
        ctx->addMenuItem("My Plugin Action", []{ qDebug("Action!"); });
        ctx->log("MyPlugin initialized");
        return true;
    }

    void shutdown() override {}
};
```

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(my_plugin)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
add_library(my_plugin SHARED my_plugin.cpp)
target_link_libraries(my_plugin PRIVATE Qt6::Core)
target_include_directories(my_plugin PRIVATE ${CMAKE_SOURCE_DIR}/../../../)
```

### Build

```bash
cd addons/plugins/my_plugin
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
# Copy .so/.dll to addons/plugins/
cp build/libmy_plugin.so .
```

### PluginContext API

```cpp
// Register a formula (QVariant signature — supports all return types)
ctx->registerFormula("MYFUNC",
    [](const QVector<QVariant> &args, OpenSheet::ParseContext &) -> QVariant {
        if (args.size() < 2) throw OpenSheet::CellError::Value;
        return args[0].toDouble() + args[1].toDouble();
    });

// Register a simple numeric formula (double-only, simpler syntax)
ctx->registerFormula("SQUARE",
    [](const QVector<double> &args) -> double {
        return args.empty() ? 0.0 : args[0] * args[0];
    });

// Add a menu item under "Add-ins"
ctx->addMenuItem("Analyze Selection", []{ /* do something */ });

// Write to the log file
ctx->log("Plugin action executed");

// Access the active workbook
OpenSheet::Workbook *wb = ctx->workbook();
if (wb && wb->activeSheet()) {
    wb->activeSheet()->setCell(1, 1, "Plugin was here");
}
```

---

## Python Plugin

### File Structure

```
addons/plugins/my_plugin/
├── main.py         ← required entry point
└── plugin.json     ← required manifest
```

### Minimal Plugin

```python
# main.py

def on_load(context):
    """Called once when the plugin is loaded."""
    context.register_formula("GREETING", greeting)
    context.add_menu_item("Say Hello", on_hello)
    context.log("MyPythonPlugin loaded")

def greeting(args):
    """=GREETING("World") → "Hello, World!" """
    name = str(args[0]) if args else "World"
    return f"Hello, {name}!"

def on_hello():
    print("Hello from My Python Plugin!")

def on_unload():
    """Called before the plugin is unloaded."""
    print("MyPythonPlugin unloaded")
```

```json
// plugin.json
{
  "name":           "My Python Plugin",
  "version":        "1.0.0",
  "description":    "Demonstrates a minimal Python plugin.",
  "author":         "Your Name",
  "type":           "python",
  "entry":          "main.py",
  "minAppVersion":  "1.0.0",
  "permissions":    ["formulas", "menu"]
}
```

### Python Context API

```python
def on_load(context):
    # Register a formula
    context.register_formula("DOUBLE", lambda args: args[0] * 2 if args else 0)

    # Register formula with validation
    def safe_sqrt(args):
        if not args:
            raise ValueError("SQRT requires an argument")
        v = float(args[0])
        if v < 0:
            return "#NUM!"
        import math
        return math.sqrt(v)
    context.register_formula("SAFESQRT", safe_sqrt)

    # Add menu item
    context.add_menu_item("My Action", lambda: print("Action triggered"))

    # Log
    context.log("Plugin loaded successfully")
```

### Returning Errors from Python Formulas

```python
def my_formula(args):
    if not args:
        return "#VALUE!"      # Return error string directly
    try:
        return 1.0 / float(args[0])
    except ZeroDivisionError:
        return "#DIV/0!"
```

---

## Plugin Manifest (plugin.json)

```json
{
  "name":           "Plugin Display Name",
  "version":        "1.2.3",
  "description":    "What this plugin does.",
  "author":         "Author Name or Org",
  "type":           "python",      // "python" or "cpp"
  "entry":          "main.py",     // Python: entry file. C++: not needed
  "minAppVersion":  "1.0.0",       // Minimum OpenSheet version required
  "permissions":    ["formulas", "menu", "workbook"],
  "homepage":       "https://example.com/plugin",
  "keywords":       ["math", "statistics", "data"]
}
```

### Permission Scopes

| Permission | Grants |
|------------|--------|
| `formulas` | Register custom formula functions |
| `menu` | Add items to the Add-ins menu |
| `workbook` | Read/write workbook cells directly |
| `settings` | Read/write application settings |
| `files` | Access the file system |

---

## Debugging Plugins

### C++ Plugins

- Use `ctx->log("message")` — appears in `logs/opensheet.log`
- Set `OPENSHEET_SANITIZE=address` in CMake to catch memory errors
- The plugin is loaded in-process; crashes will crash the app

### Python Plugins

- `print()` output goes to stdout/stderr (visible in terminal)
- `context.log()` writes to `logs/opensheet.log`
- Python exceptions are caught; the cell returns `#VALUE!`

### Log File

```
~/.local/share/OpenSheet/logs/opensheet.log   (Linux)
%APPDATA%\OpenSheet\logs\opensheet.log        (Windows)
~/Library/Application Support/OpenSheet/logs/ (macOS)
```

---

## Plugin Packaging

### For the Plugin Registry (future)

Structure your plugin as:

```
my-plugin-1.0.0/
├── plugin.json
├── main.py           (Python) or libmyplugin.so (C++)
├── README.md
└── LICENSE
```

Zip the folder and submit a PR to [opensheet/plugins](https://github.com/opensheet/plugins).
