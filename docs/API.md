# OpenSheet API Reference

This document covers every public API surface intended for use by plugin authors,
formula authors, and embedding applications.

---

## Table of Contents

1. [Plugin API](#plugin-api)
2. [Formula API](#formula-api)
3. [Engine API](#engine-api)
4. [File Format API](#file-format-api)
5. [Chart API](#chart-api)
6. [Settings API](#settings-api)

---

## Plugin API

### IPlugin interface (`plugins/plugin_manager.h`)

Every C++ plugin must implement `IPlugin` and export it via Qt's plugin machinery.

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual QString name()        const = 0;  // "My Plugin"
    virtual QString version()     const = 0;  // "1.2.0"
    virtual QString description() const = 0;
    virtual QString author()      const = 0;

    // Called once after the plugin is loaded.
    // Return false to abort loading (plugin will be unloaded immediately).
    virtual bool initialize(PluginContext *ctx) = 0;

    // Called before the plugin is unloaded (app close or user disable).
    virtual void shutdown() = 0;
};

Q_DECLARE_INTERFACE(OpenSheet::IPlugin, "io.opensheet.IPlugin/1.0")
```

### PluginContext (`plugins/plugin_manager.h`)

The context object passed to `initialize()`. Use it to hook into the application.

```cpp
class PluginContext {
public:
    // Access the active workbook
    Workbook *workbook() const;

    // Register a custom formula function
    // fn receives a flat list of QVariant arguments (ranges already expanded)
    void registerFormula(const QString &name,
                         std::function<QVariant(const QVector<QVariant>&,
                                                ParseContext&)> fn);

    // Simpler overload for pure numeric functions
    void registerFormula(const QString &name,
                         std::function<double(const QVector<double>&)> fn);

    // Add a menu item under the "Add-ins" menu
    void addMenuItem(const QString &label, std::function<void()> callback);

    // Log a message to the OpenSheet log file
    void log(const QString &message);
};
```

### Python Plugin API

Python plugins are scripts placed in `addons/plugins/<name>/main.py`
alongside a `plugin.json` manifest.

```python
def on_load(context):
    """Called when the plugin is loaded. context is a proxy object."""
    context.register_formula("DOUBLE", lambda args: args[0] * 2 if args else 0)
    context.add_menu_item("My Action", on_action)

def on_action():
    print("Action triggered from plugin")

def on_unload():
    """Called before the plugin is unloaded."""
    pass
```

### plugin.json manifest

```json
{
  "name":           "My Plugin",
  "version":        "1.0.0",
  "description":    "Does something useful.",
  "author":         "Your Name",
  "type":           "python",
  "entry":          "main.py",
  "minAppVersion":  "1.0.0",
  "permissions":    ["formulas", "menu"]
}
```

---

## Formula API

### Registering a custom formula (C++)

```cpp
// In PluginContext::initialize():
ctx->registerFormula("FACTORIAL", [](const QVector<QVariant> &args,
                                     OpenSheet::ParseContext &) -> QVariant {
    if (args.isEmpty()) return 0.0;
    long long n = args[0].toLongLong();
    if (n < 0 || n > 20) throw OpenSheet::CellError::Num;
    long long result = 1;
    for (long long i = 2; i <= n; ++i) result *= i;
    return static_cast<double>(result);
});
```

### ParseContext

```cpp
struct ParseContext {
    Sheet    *sheet    = nullptr;   // the sheet the formula lives on
    Workbook *workbook = nullptr;
    int       baseRow  = 0;         // row of the formula cell (for relative refs)
    int       baseCol  = 0;
    std::unordered_set<std::string> *visitedCells = nullptr; // circular ref guard
};
```

### CellError codes

| Code | Description |
|------|-------------|
| `CellError::DivZero` | `#DIV/0!` — division by zero |
| `CellError::Name`    | `#NAME?`  — unrecognised function or range name |
| `CellError::Value`   | `#VALUE!` — wrong argument type |
| `CellError::Ref`     | `#REF!`   — invalid cell reference |
| `CellError::NA`      | `#N/A`    — value not found (VLOOKUP etc.) |
| `CellError::Num`     | `#NUM!`   — invalid numeric argument |
| `CellError::Circular`| `#CIRC!`  — circular reference |

Throw a `CellError` from your formula implementation to set the cell's error state.

---

## Engine API

### Cell (`office/engine/cell.h`)

```cpp
// Read
cell.raw()          // QString — the raw user input or formula string
cell.value()        // QVariant — evaluated result
cell.type()         // CellType enum
cell.displayText()  // QString — formatted for display
cell.hasFormula()   // bool

// Write
cell.setRaw("=SUM(A1:A5)");        // triggers type detection
cell.setValue(42.0, CellType::Number);
cell.setError(CellError::DivZero);

// Format
cell.format()             // CellFormat& (mutable)
cell.setFormat(fmt);

// Metadata
cell.setComment("Note text");
cell.setHyperlink("https://…");
```

### Sheet (`office/engine/sheet.h`)

```cpp
// Cell access (1-based row and column indices)
sheet->cell(row, col)            // Cell& — creates empty cell if absent
sheet->setCell(row, col, "raw"); // set raw value string
sheet->clearCell(row, col);
sheet->hasCell(row, col);        // bool — true if non-empty

// Structure
sheet->insertRow(beforeRow);
sheet->deleteRow(row);
sheet->insertCol(beforeCol);
sheet->deleteCol(col);

// Sorting
sheet->sortRange(CellRange(r1,c1,r2,c2), byCol, ascending);

// Filtering
sheet->setAutoFilter(AutoFilter{.active=true, .col=1, .allowedValues={"A","B"}});
sheet->isRowHidden(row);         // bool

// Conditional formatting
ConditionalRule rule;
rule.type   = ConditionalRule::Type::GreaterThan;
rule.value1 = 100;
rule.range  = CellRange(1,1,50,5);
rule.applyFormat.foreground = Qt::green;
sheet->addConditionalRule(rule);
std::optional<CellFormat> fmt = sheet->evalConditionalFormat(row, col);

// Freeze panes
sheet->setFreezeRow(2);
sheet->setFreezeCol(1);

// Iteration
sheet->forEachCell([](int r, int c, Cell &cell) {
    qDebug() << r << c << cell.displayText();
});
```

### Workbook (`office/engine/workbook.h`)

```cpp
// Sheets
Sheet *s = wb->addSheet("Name");
wb->insertSheet(index, "Name");
wb->removeSheet(index);
wb->moveSheet(from, to);
wb->sheet(index);           // Sheet*
wb->sheet("Name");          // Sheet* (case-insensitive)
wb->activeSheet();          // Sheet*
wb->setActiveSheet(index);
wb->sheetCount();           // int
wb->sheetNames();           // QStringList

// Named ranges
wb->setNamedRange("Sales", sheet, CellRange(1,1,10,5));
Sheet *s; CellRange r;
bool ok = wb->resolveNamedRange("Sales", s, r);

// Undo/Redo
wb->undoStack()->undo();
wb->undoStack()->redo();
wb->undoStack()->push(new MyCellEditCommand(…));

// Recalculation
wb->recalcAll();
wb->recalcSheet(sheet);
```

### CellRange (`office/engine/cell_range.h`)

```cpp
CellRange range(1, 1, 10, 5);  // top, left, bottom, right (1-based)
range.top();    range.left();
range.bottom(); range.right();
range.rowCount(); range.colCount();
range.contains(row, col);       // bool
range.toString();               // "A1:E10"
CellRange r = CellRange::fromString("B2:D8");
```

---

## File Format API

### FileManager (`office/core/file_manager.h`)

```cpp
FileManager fm;

// Open any supported format (auto-detected from extension)
Workbook *wb = fm.open("/path/to/file.xlsx");
if (!wb) qWarning() << fm.lastError();

// Save
bool ok = fm.save(wb, "/path/to/output.opensheet");
if (!ok) qWarning() << fm.lastError();
```

### Supported formats

| Extension | Class | Notes |
|-----------|-------|-------|
| `.opensheet` | `OpenSheetFormat` | Native ZIP+JSON; lossless round-trip |
| `.xlsx` | `XlsxReader` / `XlsxWriter` | OOXML; shared strings, styles |
| `.csv` | `CsvReader` / `CsvWriter` | RFC 4180; auto delimiter detection |

---

## Chart API

### Creating a chart (`office/charts/chart_base.h`)

```cpp
// Factory
ChartBase *chart = ChartRenderer::create(ChartType::Bar, parentWidget);

// Configure
ChartConfig cfg;
cfg.type  = ChartType::Line;
cfg.title = "Monthly Sales";
cfg.legend = LegendPos::Bottom;

ChartSeries s;
s.name    = "Product A";
s.yValues = {120, 150, 90, 200, 175};
s.labels  = {"Jan", "Feb", "Mar", "Apr", "May"};
cfg.series.append(s);

chart->setConfig(cfg);

// Export
chart->exportPng("/path/output.png", 800, 500);
QImage img = chart->toImage(800, 500);
```

### Chart types

| Enum | Description |
|------|-------------|
| `ChartType::Bar` | Horizontal grouped bars |
| `ChartType::Line` | Connected line with data points |
| `ChartType::Pie` | Proportional pie/donut |
| `ChartType::Scatter` | X-Y scatter plot |
| `ChartType::Area` | Filled area under line |

---

## Settings API

### SettingsManager (`office/core/settings_manager.h`)

```cpp
SettingsManager settings;
settings.load();

// Read
QString theme = settings.theme();        // "light" or "dark"
bool grid     = settings.showGridLines();
int rowH      = settings.defaultRowHeight();

// Write
settings.setValue("theme", "dark");
settings.setValue("showGridLines", false);
settings.save();

// Listen for changes
connect(&settings, &SettingsManager::settingChanged,
        this, [](const QString &key, const QVariant &val) {
    qDebug() << "Setting changed:" << key << "→" << val;
});
```

---

## AutoFill API

### AutoFill (`office/engine/auto_fill.h`)

```cpp
AutoFill filler;

// Fill B2:B10 based on seed values in B1:B2
CellRange seed(1, 2, 2, 2);    // B1:B2
CellRange fill(3, 2, 10, 2);   // B3:B10
filler.fill(sheet, seed, fill, AutoFill::FillType::AutoDetect);

// Detect fill type from values
QVector<QString> seeds = {"January", "February"};
auto type = AutoFill::detectType(seeds);  // → FillType::Series
```

---

## NumberFormatter API

### NumberFormatter (`office/engine/number_formatter.h`)

```cpp
// Format a number
QString display = NumberFormatter::format(QVariant(1234567.89), "#,##0.00");
// → "1,234,567.89"

// With color
QString color;
display = NumberFormatter::format(QVariant(-50.0), "[Red]0.00", &color);
// display = "-50.00", color = "red"

// Detect a good format for a raw string
QString fmt = NumberFormatter::guessFormat("1,234.56");  // → "#,##0.00"
```

---

## PivotTable API

### PivotTable (`office/engine/pivot_table.h`)

```cpp
PivotTable pivot;
pivot.setSourceRange(dataSheet, CellRange(1, 1, 100, 4));
pivot.setHasHeaderRow(true);
pivot.setRowField({.sourceCol = 1, .label = "Region"});
pivot.setColField({.sourceCol = 2, .label = "Product"});
pivot.addValueField({.sourceCol = 3, .label = "Sales", .func = PivotTable::AggFunc::Sum});
pivot.setGrandTotals(true, true);

// Write output to another sheet
pivot.build(resultSheet, 1, 1);

// Access computed data
auto result = pivot.result();
for (const QString &rk : result.rowKeys)
    for (const QString &ck : result.colKeys)
        qDebug() << rk << ck << result.data[rk][ck];
```
