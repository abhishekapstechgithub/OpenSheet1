# OpenSheet Formula Reference

All built-in formulas supported by OpenSheet 1.0.

---

## Syntax

Formulas start with `=`. Arguments are separated by `,` (comma) or `;` (semicolon, EU locale).
Cell references use column letter + row number (e.g. `B3`, `AA10`).
Ranges use colon notation: `A1:C5`.
Absolute references use `$`: `$A$1`, `A$1`, `$A1`.
Cross-sheet references: `Sheet2!B3` or `'My Sheet'!B3`.

---

## Math & Trigonometry

| Formula | Syntax | Description |
|---------|--------|-------------|
| `SUM` | `SUM(n1, [n2], …)` | Sum of all arguments and ranges |
| `ROUND` | `ROUND(n, digits)` | Round to specified decimal places |
| `ROUNDUP` | `ROUNDUP(n, digits)` | Always round away from zero |
| `ROUNDDOWN` | `ROUNDDOWN(n, digits)` | Always round toward zero |
| `ABS` | `ABS(n)` | Absolute value |
| `MOD` | `MOD(n, divisor)` | Remainder after division |
| `POWER` | `POWER(n, exp)` | n raised to the power exp |
| `SQRT` | `SQRT(n)` | Positive square root |
| `EXP` | `EXP(n)` | e raised to the power n |
| `LN` | `LN(n)` | Natural logarithm |
| `LOG` | `LOG(n, [base])` | Logarithm (default base 10) |
| `LOG10` | `LOG10(n)` | Base-10 logarithm |
| `PI` | `PI()` | Value of π (3.14159…) |
| `SIN` | `SIN(angle)` | Sine (angle in radians) |
| `COS` | `COS(angle)` | Cosine |
| `TAN` | `TAN(angle)` | Tangent |
| `RADIANS` | `RADIANS(deg)` | Degrees to radians |
| `DEGREES` | `DEGREES(rad)` | Radians to degrees |
| `CEILING` | `CEILING(n, sig)` | Round up to nearest multiple of sig |
| `FLOOR` | `FLOOR(n, sig)` | Round down to nearest multiple |
| `INT` | `INT(n)` | Round down to nearest integer |
| `TRUNC` | `TRUNC(n, [digits])` | Truncate toward zero |
| `RAND` | `RAND()` | Random number 0 ≤ x < 1 |
| `RANDBETWEEN` | `RANDBETWEEN(low, high)` | Random integer in range |
| `SIGN` | `SIGN(n)` | -1, 0, or 1 |
| `SUMPRODUCT` | `SUMPRODUCT(a1, a2, …)` | Sum of products of ranges |
| `SUMIF` | `SUMIF(range, criteria, [sum_range])` | Conditional sum |
| `SUMIFS` | `SUMIFS(sum, r1, c1, [r2, c2]…)` | Multi-criteria sum |

---

## Statistical

| Formula | Syntax | Description |
|---------|--------|-------------|
| `AVERAGE` | `AVERAGE(n1, [n2], …)` | Arithmetic mean |
| `AVERAGEIF` | `AVERAGEIF(range, criteria, [avg_range])` | Conditional average |
| `COUNT` | `COUNT(v1, [v2], …)` | Count numeric values |
| `COUNTA` | `COUNTA(v1, [v2], …)` | Count non-empty cells |
| `COUNTBLANK` | `COUNTBLANK(range)` | Count empty cells |
| `COUNTIF` | `COUNTIF(range, criteria)` | Conditional count |
| `COUNTIFS` | `COUNTIFS(r1, c1, [r2, c2]…)` | Multi-criteria count |
| `MIN` | `MIN(n1, [n2], …)` | Smallest value |
| `MAX` | `MAX(n1, [n2], …)` | Largest value |
| `MEDIAN` | `MEDIAN(n1, [n2], …)` | Median value |
| `MODE` | `MODE(n1, [n2], …)` | Most frequent value |
| `STDEV` | `STDEV(n1, [n2], …)` | Sample standard deviation |
| `STDEVP` | `STDEVP(n1, [n2], …)` | Population standard deviation |
| `VAR` | `VAR(n1, [n2], …)` | Sample variance |
| `VARP` | `VARP(n1, [n2], …)` | Population variance |
| `LARGE` | `LARGE(range, k)` | k-th largest value |
| `SMALL` | `SMALL(range, k)` | k-th smallest value |
| `RANK` | `RANK(n, range, [order])` | Rank of n in range |
| `PERCENTILE` | `PERCENTILE(range, k)` | k-th percentile (0–1) |
| `QUARTILE` | `QUARTILE(range, quart)` | Quartile (0–4) |
| `CORREL` | `CORREL(a, b)` | Pearson correlation coefficient |

---

## Logical

| Formula | Syntax | Description |
|---------|--------|-------------|
| `IF` | `IF(test, true_val, [false_val])` | Conditional evaluation |
| `IFS` | `IFS(test1, v1, test2, v2, …)` | Multiple conditions |
| `AND` | `AND(l1, [l2], …)` | TRUE if all arguments TRUE |
| `OR` | `OR(l1, [l2], …)` | TRUE if any argument TRUE |
| `NOT` | `NOT(logical)` | Reverse TRUE/FALSE |
| `XOR` | `XOR(l1, l2, …)` | TRUE if odd number of TRUE args |
| `IFERROR` | `IFERROR(value, fallback)` | Return fallback on any error |
| `IFNA` | `IFNA(value, fallback)` | Return fallback on #N/A |
| `TRUE` | `TRUE()` | Logical TRUE |
| `FALSE` | `FALSE()` | Logical FALSE |
| `SWITCH` | `SWITCH(expr, v1, r1, [v2, r2]…, [default])` | Match and return |

---

## Text

| Formula | Syntax | Description |
|---------|--------|-------------|
| `CONCATENATE` | `CONCATENATE(t1, [t2], …)` | Join text strings |
| `CONCAT` | `CONCAT(t1, [t2], …)` | Alias for CONCATENATE |
| `TEXTJOIN` | `TEXTJOIN(delim, ignore_empty, t1, …)` | Join with delimiter |
| `LEN` | `LEN(text)` | Number of characters |
| `UPPER` | `UPPER(text)` | Convert to uppercase |
| `LOWER` | `LOWER(text)` | Convert to lowercase |
| `PROPER` | `PROPER(text)` | Title Case |
| `LEFT` | `LEFT(text, [n])` | First n characters |
| `RIGHT` | `RIGHT(text, [n])` | Last n characters |
| `MID` | `MID(text, start, n)` | n chars from position start |
| `TRIM` | `TRIM(text)` | Remove leading/trailing/extra spaces |
| `CLEAN` | `CLEAN(text)` | Remove non-printable characters |
| `SUBSTITUTE` | `SUBSTITUTE(text, old, new, [n])` | Replace text occurrences |
| `REPLACE` | `REPLACE(text, start, n, new)` | Replace by position |
| `FIND` | `FIND(find, within, [start])` | Case-sensitive position search |
| `SEARCH` | `SEARCH(find, within, [start])` | Case-insensitive position search |
| `TEXT` | `TEXT(value, format)` | Format number as text |
| `VALUE` | `VALUE(text)` | Convert text to number |
| `T` | `T(value)` | Return text if text, else empty |
| `REPT` | `REPT(text, n)` | Repeat text n times |
| `EXACT` | `EXACT(t1, t2)` | Case-sensitive equality |
| `CODE` | `CODE(text)` | Character code of first char |
| `CHAR` | `CHAR(n)` | Character from code |

---

## Date & Time

| Formula | Syntax | Description |
|---------|--------|-------------|
| `TODAY` | `TODAY()` | Current date serial |
| `NOW` | `NOW()` | Current date+time serial |
| `DATE` | `DATE(year, month, day)` | Create a date serial |
| `TIME` | `TIME(h, m, s)` | Create a time serial |
| `YEAR` | `YEAR(date)` | Extract year |
| `MONTH` | `MONTH(date)` | Extract month (1–12) |
| `DAY` | `DAY(date)` | Extract day of month (1–31) |
| `HOUR` | `HOUR(time)` | Extract hour (0–23) |
| `MINUTE` | `MINUTE(time)` | Extract minute (0–59) |
| `SECOND` | `SECOND(time)` | Extract second (0–59) |
| `WEEKDAY` | `WEEKDAY(date, [type])` | Day of week (1–7) |
| `WEEKNUM` | `WEEKNUM(date, [type])` | Week number of the year |
| `DATEVALUE` | `DATEVALUE(text)` | Convert date text to serial |
| `TIMEVALUE` | `TIMEVALUE(text)` | Convert time text to serial |
| `DAYS` | `DAYS(end, start)` | Days between two dates |
| `DAYS360` | `DAYS360(start, end, [method])` | Days on 360-day year |
| `EDATE` | `EDATE(start, months)` | Date n months away |
| `EOMONTH` | `EOMONTH(start, months)` | Last day of month |
| `NETWORKDAYS` | `NETWORKDAYS(start, end, [holidays])` | Working days between dates |

---

## Lookup & Reference

| Formula | Syntax | Description |
|---------|--------|-------------|
| `VLOOKUP` | `VLOOKUP(val, range, col, [exact])` | Vertical lookup |
| `HLOOKUP` | `HLOOKUP(val, range, row, [exact])` | Horizontal lookup |
| `INDEX` | `INDEX(range, row, [col])` | Return value at position |
| `MATCH` | `MATCH(val, range, [type])` | Position of value in range |
| `OFFSET` | `OFFSET(ref, rows, cols, [h], [w])` | Reference offset from ref |
| `CHOOSE` | `CHOOSE(index, v1, v2, …)` | Choose from list by index |
| `ROW` | `ROW([ref])` | Row number of reference |
| `COLUMN` | `COLUMN([ref])` | Column number of reference |
| `ROWS` | `ROWS(range)` | Number of rows in range |
| `COLUMNS` | `COLUMNS(range)` | Number of columns in range |
| `INDIRECT` | `INDIRECT(text, [a1])` | Reference from text string |
| `ADDRESS` | `ADDRESS(row, col, [abs], [a1], [sheet])` | Build a reference string |
| `TRANSPOSE` | `TRANSPOSE(range)` | Swap rows and columns |
| `LOOKUP` | `LOOKUP(val, range)` | Approximate match lookup |
| `XLOOKUP` | `XLOOKUP(val, lookup, return, [miss], [match], [search])` | Modern lookup |

---

## Information

| Formula | Syntax | Description |
|---------|--------|-------------|
| `ISNUMBER` | `ISNUMBER(v)` | TRUE if v is numeric |
| `ISTEXT` | `ISTEXT(v)` | TRUE if v is text |
| `ISBLANK` | `ISBLANK(v)` | TRUE if v is empty |
| `ISERROR` | `ISERROR(v)` | TRUE if v is any error |
| `ISERR` | `ISERR(v)` | TRUE if v is error except #N/A |
| `ISNA` | `ISNA(v)` | TRUE if v is #N/A |
| `ISLOGICAL` | `ISLOGICAL(v)` | TRUE if v is TRUE or FALSE |
| `ISFORMULA` | `ISFORMULA(ref)` | TRUE if cell contains a formula |
| `NA` | `NA()` | Return #N/A error |
| `ERROR.TYPE` | `ERROR.TYPE(v)` | Number code for error type |
| `TYPE` | `TYPE(v)` | 1=number, 2=text, 4=bool, 16=error |
| `CELL` | `CELL(info_type, [ref])` | Information about a cell |

---

## Error Codes

| Code | Meaning |
|------|---------|
| `#DIV/0!` | Division by zero |
| `#NAME?` | Unrecognised function or name |
| `#VALUE!` | Wrong argument type |
| `#REF!` | Invalid cell reference |
| `#N/A` | Value not available |
| `#NULL!` | Invalid intersection |
| `#NUM!` | Invalid numeric value |
| `#CIRC!` | Circular reference (OpenSheet-specific) |
