# Changelog

All notable changes to OpenSheet are documented here.
Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versioning follows [Semantic Versioning](https://semver.org/).

---

## [Unreleased]

### Added
- GitHub Actions CI/CD pipeline (Linux, Windows, macOS)
- CodeQL security scanning
- Nightly builds with Valgrind memory checks and code coverage

---

## [1.0.0] — 2024-01-01

### Added
- Full spreadsheet grid (500 rows × 50 cols, sparse storage)
- Multiple sheets per workbook with tab management
- Formula engine with 30+ built-in functions:
  `SUM`, `AVERAGE`, `COUNT`, `COUNTA`, `MIN`, `MAX`,
  `IF`, `AND`, `OR`, `NOT`, `IFERROR`,
  `VLOOKUP`, `HLOOKUP`, `INDEX`, `MATCH`,
  `SUMIF`, `COUNTIF`,
  `CONCATENATE`, `LEN`, `UPPER`, `LOWER`, `LEFT`, `RIGHT`, `MID`, `TRIM`,
  `ROUND`, `ABS`, `MOD`, `POWER`, `SQRT`,
  `TODAY`, `NOW`, `YEAR`, `MONTH`, `DAY`
- Circular reference detection (`#CIRC!`)
- Dependency graph with topological recalculation order
- Cell types: Number, Text, Boolean, Date, Formula, Error
- Cell formatting: font, bold/italic/underline, colors, alignment, borders, number formats
- Conditional formatting rules
- Sort (A→Z / Z→A) on any column
- Auto-filter with value matching
- Freeze panes (rows and columns)
- File formats: `.opensheet` (native ZIP/JSON), `.xlsx` (OOXML), `.csv`
- Undo / Redo stack (100 levels)
- Auto-save every 60 seconds
- Crash recovery: detects unsaved sessions on restart
- Ribbon-style toolbar (Home, Insert, Page Layout, Formulas, Data, Review, View)
- Formula bar with cell reference navigator
- Sheet tabs with add / rename / delete / reorder
- Status bar with Sum / Average / Count for selection
- Side panel: cell info, sheet stats, named ranges, quick actions
- Chart engine: Bar, Line, Pie, Scatter, Area — QPainter-based, no 3rd-party lib
- Plugin system: C++ shared libraries (QPluginLoader) + Python scripts
- Example Python plugin (`addons/plugins/hello_plugin`)
- Light and Dark themes (QSS)
- Localization: English (en-US), French (fr-FR), German (de-DE)
- Configurable settings (`config/opensheet.json`)
- Full Qt Test unit test suite (engine, file I/O, charts)
- CMake build system with cross-platform support

### Architecture
- Engine layer: `Cell`, `Sheet`, `Workbook`, `FormulaParser`, `DependencyGraph`
- Core layer: `FileManager`, `XlsxReader`, `CsvReader`, `OpenSheetFormat`, `SettingsManager`, `CrashRecovery`
- UI layer: `MainWindow`, `RibbonBar`, `SpreadsheetView`, `FormulaBar`, `SheetTabs`, `StatusBar`, `SidePanel`
- Charts: `ChartBase` + 5 concrete chart types + `ChartRenderer` factory
- Plugins: `PluginManager`, `IPlugin` interface, `PluginContext` facade

---

[Unreleased]: https://github.com/opensheet/opensheet/compare/v1.0.0...HEAD
[1.0.0]:      https://github.com/opensheet/opensheet/releases/tag/v1.0.0
