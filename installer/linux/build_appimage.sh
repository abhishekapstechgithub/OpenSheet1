#!/usr/bin/env bash
# build_appimage.sh — Packages OpenSheet as a portable Linux AppImage
# Usage: ./build_appimage.sh [build_dir] [output_dir]
# Requires: linuxdeploy, linuxdeploy-plugin-qt, appimagetool

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${1:-$PROJECT_ROOT/build}"
OUTPUT_DIR="${2:-$PROJECT_ROOT/dist}"
APP_VERSION="${APP_VERSION:-1.0.0}"
APP_NAME="OpenSheet"
APPDIR="$OUTPUT_DIR/AppDir"

echo "=== OpenSheet AppImage Builder ==="
echo "  Project: $PROJECT_ROOT"
echo "  Build:   $BUILD_DIR"
echo "  Output:  $OUTPUT_DIR"
echo "  Version: $APP_VERSION"
echo ""

# ── Prerequisites ─────────────────────────────────────────────────────────────
check_tool() {
    if ! command -v "$1" &>/dev/null; then
        echo "ERROR: '$1' not found. Install it and retry."
        echo "  → https://github.com/linuxdeploy/linuxdeploy"
        exit 1
    fi
}

check_tool linuxdeploy
check_tool linuxdeploy-plugin-qt
check_tool appimagetool

# ── Prepare AppDir ────────────────────────────────────────────────────────────
rm -rf   "$APPDIR"
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/$APP_NAME"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPDIR/usr/share/icons/hicolor/512x512/apps"

echo "[1/6] Copying binary…"
cp "$BUILD_DIR/bin/opensheet" "$APPDIR/usr/bin/opensheet"
chmod +x "$APPDIR/usr/bin/opensheet"

echo "[2/6] Copying app data…"
for dir in themes localization resources addons config; do
    src="$PROJECT_ROOT/$dir"
    if [ -d "$src" ]; then
        cp -r "$src" "$APPDIR/usr/share/$APP_NAME/"
    fi
done

echo "[3/6] Writing .desktop file…"
cat > "$APPDIR/usr/share/applications/opensheet.desktop" << EOF
[Desktop Entry]
Name=$APP_NAME
GenericName=Spreadsheet
Comment=Professional open-source spreadsheet application
Exec=opensheet %F
Icon=opensheet
Terminal=false
Type=Application
Categories=Office;Spreadsheet;
MimeType=application/vnd.opensheet;application/vnd.openxmlformats-officedocument.spreadsheetml.sheet;text/csv;
Keywords=spreadsheet;excel;calc;office;
StartupNotify=true
StartupWMClass=opensheet
EOF

echo "[4/6] Copying icons…"
ICON_SRC="$PROJECT_ROOT/resources/icons/logo.png"
if [ -f "$ICON_SRC" ]; then
    cp "$ICON_SRC" "$APPDIR/usr/share/icons/hicolor/256x256/apps/opensheet.png"
    cp "$ICON_SRC" "$APPDIR/usr/share/icons/hicolor/512x512/apps/opensheet.png"
    cp "$ICON_SRC" "$APPDIR/opensheet.png"
fi

echo "[5/6] Running linuxdeploy (bundles Qt + system libs)…"
export QMAKE="${QMAKE:-qmake6}"
export OUTPUT="$OUTPUT_DIR/$APP_NAME-$APP_VERSION-x86_64.AppImage"

linuxdeploy \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/opensheet" \
    --desktop-file "$APPDIR/usr/share/applications/opensheet.desktop" \
    --icon-file "$APPDIR/opensheet.png" \
    --plugin qt \
    --output appimage

echo "[6/6] Done."
echo ""
echo "AppImage: $OUTPUT"
echo "Size: $(du -sh "$OUTPUT" | cut -f1)"
ls -lh "$OUTPUT"
