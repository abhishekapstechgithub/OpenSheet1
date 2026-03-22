# OpenSheet Architecture

## Overview

OpenSheet is a layered application. Each layer has a single responsibility and communicates with adjacent layers via clean interfaces.

```
┌─────────────────────────────────────────────────────────────┐
│                     Qt6 Application Layer                    │
│  main.cpp │ QApplication │ QSettings │ CrashRecovery        │
├─────────────────────────────────────────────────────────────┤
│                       UI Layer (Qt Widgets)                  │
│  MainWindow │ RibbonBar │ SpreadsheetView │ FormulaBar       │
│  SheetTabs  │ StatusBar │ SidePanel       │ Dialogs          │
├─────────────────────────────────────────────────────────────┤
│                      Core / File Layer                       │
│  FileManager │ XlsxReader │ CsvReader │ OpenSheetFormat     │
│  SettingsManager │ CrashRecovery │ AutoSave                 │
├─────────────────────────────────────────────────────────────┤
│                     Spreadsheet Engine                       │
│  Workbook │ Sheet │ Cell │ FormulaParser │ DependencyGraph   │
│  RecalcEngine │ CellRange │ NamedRanges                     │
├─────────────────────────────────────────────────────────────┤
│              Charts │ Formulas │ Plugin System               │
│  ChartBase │ BarChart │ LineChart │ PieChart │ ScatterChart  │
│  FormulaRegistry │ PluginManager │ IPlugin │ PluginContext  │
└─────────────────────────────────────────────────────────────┘
```

---

## Engine Layer

### Cell (`cell.h`)

The atomic unit. Stores:
- `raw` — the user-entered string (or formula string starting with `=`)
- `value` — the evaluated QVariant (Number, String, Bool, Date, or Error)
- `type` — CellType enum (Empty, Text, Number, Boolean, Date, Formula, Error)
- `format` — CellFormat struct (font, colors, alignment, borders, numberFormat)
- `comment`, `hyperlink` — metadata
- merge span for merged cell regions

Type detection is automatic when `setRaw()` is called: number strings become
`CellType::Number`, ISO date strings become `CellType::Date`, etc.

### Sheet (`sheet.h`)

A sparse 2D grid of cells backed by `QHash<CellAddress, Cell>`. Key design decisions:

- **Sparse storage**: Only non-empty cells consume memory. A sheet of 500×50
  with 100 filled cells stores exactly 100 Cell objects.
- **Row/Col properties** (height, width, hidden) stored separately in small hashes.
- **Insert/Delete** rows and columns shift all keys above/right by re-hashing.
- **Conditional formatting** rules are stored as a list of `ConditionalRule`,
  evaluated lazily at paint time by `SpreadsheetView`.
- **Auto-filter** hides rows that don't match the filter values.

### Workbook (`workbook.h`)

Owns a `QVector<Sheet*>` and coordinates:
- Active sheet index
- `QUndoStack` shared across all sheets
- Named range registry
- Triggers `RecalcEngine` after cell edits

### FormulaParser (`formula_parser.h`)

A hand-written recursive descent parser. Pipeline:

```
Raw string "=SUM(A1:B5)+IF(C1>0,1,0)"
      │
      ▼
  Tokenizer → [=, SUM, (, A1, :, B5, ), +, IF, (, C1, >, 0, ,, 1, ,, 0, )]
      │
      ▼
  parseExpr() → parseTerm() → parseFactor() → parseAtom()
      │               │
      │       parseFunction("SUM", args=[range A1:B5])
      │
      ▼
  QVariant result (e.g., 42.0)
```

Cell references inside formulas (`A1`, `$B$3`, `Sheet2!C4`) are resolved
through `ParseContext`, which carries the current Sheet and Workbook.
Circular references are detected via a `visited` set passed through the
recursion.

### DependencyGraph (`dependency_graph.h`)

Maintains two directed adjacency maps:
- `m_graph[dependency]  = set<dependents>`  — who depends on this cell
- `m_reverse[dependent] = set<dependencies>` — what this cell depends on

`topoSort()` returns cells in a safe recalculation order (dependencies before
dependents) using DFS. Returns `false` if a cycle is detected, allowing
the engine to mark circular cells with `#CIRC!`.

---

## File Formats

### .opensheet (Native)

A ZIP archive (uses Qt's QZipWriter/QZipReader) containing JSON files:

```
workbook.opensheet  (ZIP)
├── manifest.json       { version, sheetCount, sheetNames, activeSheet }
├── sheets/0.json       { name, cells:[{r,c,raw,fmt,...}], freezeRow, ... }
├── sheets/1.json
├── styles.json         { global style table (future) }
├── namedRanges.json    [ {name, sheet, range} ]
└── media/              (embedded images, future)
```

Each cell is serialized only if it's non-empty. A 1000-row sheet with 20
filled cells produces a 20-cell JSON array, not a 1000-row array.

### .xlsx (OOXML)

Parsed via Qt's XML stack (QXmlStreamReader for reading, QXmlStreamWriter
for writing). Key files inside the xlsx ZIP:
- `xl/workbook.xml` — sheet list
- `xl/worksheets/sheet1.xml` — cell data (`<c r="B3" t="s"><v>5</v></c>`)
- `xl/sharedStrings.xml` — string pool (index referenced by `t="s"`)
- `xl/styles.xml` — number formats, fonts, fills, borders

### .csv

- Delimiter auto-detection from the first line (comma, semicolon, tab, pipe)
- RFC 4180 quoting: fields with delimiters or newlines wrapped in `"`
- Reads into a single-sheet workbook; writes the active sheet

---

## UI Layer

### SpreadsheetView

Renders the grid using direct `QPainter` calls rather than QTableWidget.
This gives full control over performance and appearance.

Paint cycle (per `paintEvent`):
1. Calculate visible row/col range from scroll position
2. Draw column headers (sticky top)
3. Draw row numbers (sticky left)
4. For each visible cell:
   a. Fill background (normal / selected / in-range / conditional format)
   b. Draw text with alignment, number format, font
   c. Draw borders
5. Draw selection highlight & fill handle
6. Draw freeze pane lines

**Inline editing**: A `QLineEdit` is overlaid on the active cell, positioned
absolutely via `move()`. On commit (Enter/Tab/click-away), the value is
written back to the Sheet and the formula bar is updated.

### RibbonBar

A `QTabBar` + `QStackedWidget` pair. Each tab page is a `QWidget` with a
`QHBoxLayout` of ribbon groups. Ribbon groups are `QFrame` widgets with a
`QVBoxLayout` (buttons above, group label below, separator line on right).

---

## Plugin System

```
PluginManager::scanAndLoad(dirPath)
    │
    ├─ *.so / *.dll  ──► QPluginLoader ──► IPlugin::initialize(ctx)
    │
    └─ subdirs with plugin.json + main.py
                         └──► Python3 C API ──► PyRun_SimpleFile(main.py)
                                                 calls on_load(context_proxy)
```

`PluginContext` is a thin facade that routes plugin calls to the Workbook
without exposing the full internal API. This keeps the plugin API stable
even as internals change.

---

## Threading Model

- All UI operations run on the main thread (Qt requirement)
- `RecalcEngine` can offload heavy recalculation to a `QThread` with a
  worker object; cell value results are posted back to the main thread
  via `QMetaObject::invokeMethod` / `Qt::QueuedConnection`
- File I/O (especially large .xlsx) runs on a `QThreadPool` worker;
  progress is reported via signals

---

## Memory Management

- `Workbook` owns `Sheet` objects (QObject parent-child)
- `Sheet` owns no heap allocations beyond `QHash` internals
- `PluginManager` owns loaded plugin instances
- `FormulaParser` is stack-allocated per-evaluation call (no global state)
- `DependencyGraph` is owned by `RecalcEngine` which is owned by `Workbook`

---

## Extension Points

| Area            | How to extend                                         |
|-----------------|-------------------------------------------------------|
| File formats    | Subclass no interface; add a branch in `FileManager`  |
| Chart types     | Subclass `ChartBase`, register in `ChartRenderer`     |
| Formula functions | `FormulaParser::registerFunction()` or Python plugin |
| UI themes       | Add a new `.qss` file and entry in settings           |
| Localization    | Add a `strings.json` in `localization/<lang>/`        |
| Plugins         | C++ shared lib implementing `IPlugin`, or Python script |
