# OpenSheet

A professional, open-source spreadsheet application built with **C++20** and **Qt6**.

## Features

- Full spreadsheet grid with 500+ rows × 50+ columns
- Multiple sheets per workbook with tab management
- Formula engine: SUM, AVERAGE, COUNT, MIN, MAX, IF, VLOOKUP, HLOOKUP, and 30+ more
- Circular reference detection
- Dependency graph for intelligent recalculation
- File formats: `.opensheet` (native), `.xlsx` (Excel), `.csv`
- Undo / Redo (100 levels)
- Auto-save & crash recovery
- Conditional formatting
- Sort & filter
- Freeze panes
- Charts: Bar, Line, Pie, Scatter, Area
- Plugin system (C++ shared libs + Python scripts)
- Light / Dark themes
- Localization: English, French, German (extensible)
- Ribbon-style toolbar
- Formula bar, sheet tabs, status bar, side panel

---

## Project Structure

```
OpenSheet/
├── main.cpp                  Entry point
├── CMakeLists.txt            Root build configuration
├── cmake/                    CMake helper modules
├── office/
│   ├── engine/               Core spreadsheet engine
│   │   ├── cell.h/cpp        Cell data model & type detection
│   │   ├── cell_range.h      Range address helpers
│   │   ├── sheet.h/cpp       Sheet (grid of cells)
│   │   ├── workbook.h/cpp    Workbook (collection of sheets)
│   │   ├── formula_parser.h  Formula tokenizer & evaluator
│   │   ├── dependency_graph.h Recalc dependency tracking
│   │   └── recalc_engine.h   Full recalculation engine
│   ├── core/
│   │   ├── file_system/      File I/O handlers
│   │   │   ├── file_manager  Format dispatcher
│   │   │   ├── xlsx_reader   .xlsx read/write
│   │   │   ├── csv_reader    .csv read/write
│   │   │   └── opensheet_format  Native .opensheet format
│   │   ├── settings_manager  Persistent app settings
│   │   └── crash_recovery    Session recovery on restart
│   ├── ui/                   Qt6 UI components
│   │   ├── main_window       Application main window
│   │   ├── ribbon_bar        Ribbon toolbar (7 tabs)
│   │   ├── spreadsheet_view  High-performance grid widget
│   │   ├── formula_bar       Formula input bar
│   │   ├── sheet_tabs        Workbook sheet tab bar
│   │   ├── status_bar        Status / statistics bar
│   │   └── side_panel        Properties side panel
│   ├── charts/               Chart rendering engine
│   │   └── chart_base        Base + Bar/Line/Pie/Scatter/Area
│   └── formulas/             Extended formula registry
├── plugins/                  Plugin manager & API
├── addons/
│   ├── plugins/              Third-party plugins (C++, Python)
│   └── templates/            Workbook templates
├── themes/
│   ├── light/style.qss       Light theme
│   └── dark/style.qss        Dark theme
├── localization/             i18n JSON files
├── resources/                Icons, fonts, QRC
├── tests/                    Qt Test unit tests
├── docs/                     Documentation
├── config/                   Default application config
├── logs/                     Runtime log files
└── installer/                Platform installer scripts
```

---

## Building

### Prerequisites

| Dependency  | Version  | Notes                     |
|-------------|----------|---------------------------|
| CMake       | ≥ 3.20   | Build system              |
| Qt6         | ≥ 6.4    | Core, Widgets, Charts, Xml, Sql, Network |
| SQLite3     | ≥ 3.38   | Local storage             |
| C++ compiler| C++20    | GCC 12+, Clang 14+, MSVC 2022 |
| Python 3    | ≥ 3.8    | Optional (plugin support) |

### Linux / macOS

```bash
git clone https://github.com/opensheet/opensheet.git
cd opensheet

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run
./bin/opensheet
```

### Windows (Visual Studio 2022)

```powershell
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.x/msvc2022_64"
cmake --build . --config Release

.\bin\Release\opensheet.exe
```

### CMake Options

| Option                        | Default | Description                  |
|-------------------------------|---------|------------------------------|
| `CMAKE_BUILD_TYPE`            | Release | Debug / Release              |
| `OPENSHEET_PYTHON_SUPPORT`    | AUTO    | Enable Python plugin loader  |
| `OPENSHEET_BUILD_TESTS`       | ON      | Build unit tests             |
| `OPENSHEET_INSTALLER`         | OFF     | Build installer package      |

---

## Running Tests

```bash
cd build
ctest --output-on-failure

# Or run individual test binaries:
./tests/test_engine
./tests/test_fileio
./tests/test_charts
```

---

## Writing a Plugin

### C++ Plugin

```cpp
#include <plugins/plugin_manager.h>

class MyPlugin : public QObject, public OpenSheet::IPlugin {
    Q_OBJECT
    Q_INTERFACES(OpenSheet::IPlugin)
    Q_PLUGIN_METADATA(IID "io.opensheet.IPlugin/1.0")
public:
    QString name()    const override { return "My Plugin"; }
    QString version() const override { return "1.0.0"; }
    // ...
    bool initialize(OpenSheet::PluginContext *ctx) override {
        ctx->registerFormula("MYFORMULA", [](const QVector<double>& args){
            return args.empty() ? 0.0 : args[0] * 2.0;
        });
        return true;
    }
    void shutdown() override {}
};
```

Compile as a shared library and place in `addons/plugins/`.

### Python Plugin

Create `addons/plugins/myplugin/plugin.json` and `main.py`:

```python
def on_load(context):
    context.register_formula("DOUBLE", lambda args: args[0] * 2 if args else 0)
    context.add_menu_item("My Action", lambda: print("Hello!"))

def on_unload():
    pass
```

---

## File Formats

| Extension     | Description                          | Read | Write |
|---------------|--------------------------------------|------|-------|
| `.opensheet`  | Native ZIP/JSON format               | ✅   | ✅    |
| `.xlsx`       | Microsoft Excel (OOXML)              | ✅   | ✅    |
| `.csv`        | Comma-separated values               | ✅   | ✅    |

---

## Supported Formulas

`SUM` `AVERAGE` `COUNT` `COUNTA` `MIN` `MAX` `IF` `AND` `OR` `NOT`
`IFERROR` `VLOOKUP` `HLOOKUP` `INDEX` `MATCH` `SUMIF` `COUNTIF`
`CONCATENATE` `LEN` `UPPER` `LOWER` `LEFT` `RIGHT` `MID` `TRIM`
`ROUND` `ABS` `MOD` `POWER` `SQRT` `TODAY` `NOW` `YEAR` `MONTH` `DAY`

---

## License

MIT License — see [LICENSE](LICENSE) for details.

---

## Contributing

Pull requests are welcome. Please open an issue first to discuss major changes.
Run `ctest` before submitting a PR to ensure all tests pass.
