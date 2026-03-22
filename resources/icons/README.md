# OpenSheet Icon Assets

This directory contains all icon assets used by OpenSheet.

## Required Icons

| Filename | Size | Format | Purpose |
|----------|------|--------|---------|
| `logo.png` | 256×256 | PNG | App icon (all platforms) |
| `logo.svg` | scalable | SVG | Vector app icon |
| `logo.ico` | multi-size | ICO | Windows installer/taskbar |
| `logo.icns` | multi-size | ICNS | macOS app bundle |
| `splash.png` | 480×280 | PNG | Splash screen |

## Toolbar Icons (24×24 px, SVG preferred)

| Filename | Action |
|----------|--------|
| `new.svg` | New workbook |
| `open.svg` | Open file |
| `save.svg` | Save |
| `undo.svg` | Undo |
| `redo.svg` | Redo |
| `cut.svg` | Cut |
| `copy.svg` | Copy |
| `paste.svg` | Paste |
| `bold.svg` | Bold |
| `italic.svg` | Italic |
| `underline.svg` | Underline |
| `align_left.svg` | Align left |
| `align_center.svg` | Align center |
| `align_right.svg` | Align right |
| `sort_az.svg` | Sort ascending |
| `sort_za.svg` | Sort descending |
| `filter.svg` | Filter |
| `chart_bar.svg` | Bar chart |
| `chart_line.svg` | Line chart |
| `chart_pie.svg` | Pie chart |
| `sum.svg` | AutoSum |
| `function.svg` | Insert function |
| `freeze.svg` | Freeze panes |
| `darkmode.svg` | Toggle dark mode |
| `close.svg` | Close / X button |

## Sourcing Icons

Recommended free icon sets compatible with the MIT license:
- [Lucide Icons](https://lucide.dev) — clean, consistent, MIT
- [Phosphor Icons](https://phosphoricons.com) — MIT
- [Heroicons](https://heroicons.com) — MIT
- [Feather Icons](https://feathericons.com) — MIT

## Icon Generation (macOS .icns)

```bash
mkdir opensheet.iconset
for size in 16 32 64 128 256 512; do
    sips -z $size $size logo.png --out opensheet.iconset/icon_${size}x${size}.png
done
iconutil -c icns opensheet.iconset -o logo.icns
```

## Icon Generation (Windows .ico)

```bash
convert logo.png -resize 256x256 \
    \( +clone -resize 128x128 \) \
    \( +clone -resize 64x64 \) \
    \( +clone -resize 48x48 \) \
    \( +clone -resize 32x32 \) \
    \( +clone -resize 16x16 \) \
    logo.ico
```
