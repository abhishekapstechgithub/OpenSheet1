#!/usr/bin/env bash
# build_dmg.sh — Creates a distributable macOS DMG for OpenSheet
# Usage: ./build_dmg.sh [build_dir] [output_dir]
# Requires: macdeployqt, create-dmg (brew install create-dmg) or hdiutil

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${1:-$PROJECT_ROOT/build}"
OUTPUT_DIR="${2:-$PROJECT_ROOT/dist}"
APP_VERSION="${APP_VERSION:-1.0.0}"
APP_BUNDLE="$BUILD_DIR/bin/opensheet.app"
DMG_NAME="OpenSheet-$APP_VERSION-macOS"

echo "=== OpenSheet macOS DMG Builder ==="
echo "  App bundle: $APP_BUNDLE"
echo "  Output:     $OUTPUT_DIR/$DMG_NAME.dmg"
echo ""

mkdir -p "$OUTPUT_DIR"

# ── Verify app bundle ─────────────────────────────────────────────────────────
if [ ! -d "$APP_BUNDLE" ]; then
    echo "ERROR: App bundle not found at $APP_BUNDLE"
    echo "  → Run 'cmake --build build' first."
    exit 1
fi

# ── Run macdeployqt ───────────────────────────────────────────────────────────
echo "[1/4] Running macdeployqt…"
QMAKE_BIN="${QMAKE:-$(command -v qmake6 || command -v qmake)}"
QT_DIR="$(dirname "$(dirname "$QMAKE_BIN")")"
MACDEPLOYQT="$QT_DIR/bin/macdeployqt"

if [ ! -f "$MACDEPLOYQT" ]; then
    echo "ERROR: macdeployqt not found at $MACDEPLOYQT"
    exit 1
fi

"$MACDEPLOYQT" "$APP_BUNDLE" -verbose=1

# ── Copy app data into bundle ─────────────────────────────────────────────────
echo "[2/4] Embedding app data…"
RESOURCES="$APP_BUNDLE/Contents/Resources"
for dir in themes localization addons config; do
    src="$PROJECT_ROOT/$dir"
    [ -d "$src" ] && cp -r "$src" "$RESOURCES/"
done

# ── Code sign (optional) ──────────────────────────────────────────────────────
if [ -n "${APPLE_SIGNING_IDENTITY:-}" ]; then
    echo "[2b] Code signing with: $APPLE_SIGNING_IDENTITY"
    codesign --deep --force --verify --verbose \
        --sign "$APPLE_SIGNING_IDENTITY" \
        --options runtime \
        "$APP_BUNDLE"
else
    echo "[2b] Skipping code sign (APPLE_SIGNING_IDENTITY not set)"
fi

# ── Create DMG ────────────────────────────────────────────────────────────────
echo "[3/4] Creating DMG…"
DMG_STAGING="$OUTPUT_DIR/dmg-staging"
rm -rf "$DMG_STAGING"
mkdir -p "$DMG_STAGING"
cp -r "$APP_BUNDLE" "$DMG_STAGING/"
# Symlink to /Applications
ln -s /Applications "$DMG_STAGING/Applications"

hdiutil create \
    -volname "OpenSheet $APP_VERSION" \
    -srcfolder "$DMG_STAGING" \
    -ov \
    -format UDZO \
    -imagekey zlib-level=9 \
    "$OUTPUT_DIR/$DMG_NAME.dmg"

rm -rf "$DMG_STAGING"

# ── Notarize (optional) ───────────────────────────────────────────────────────
if [ -n "${APPLE_NOTARIZE_KEY:-}" ] && [ -n "${APPLE_NOTARIZE_ISSUER:-}" ]; then
    echo "[4/4] Notarizing DMG…"
    xcrun notarytool submit "$OUTPUT_DIR/$DMG_NAME.dmg" \
        --key "$APPLE_NOTARIZE_KEY" \
        --key-id "${APPLE_NOTARIZE_KEY_ID}" \
        --issuer "$APPLE_NOTARIZE_ISSUER" \
        --wait
    xcrun stapler staple "$OUTPUT_DIR/$DMG_NAME.dmg"
else
    echo "[4/4] Skipping notarization (APPLE_NOTARIZE_KEY not set)"
fi

echo ""
echo "Done."
echo "DMG: $OUTPUT_DIR/$DMG_NAME.dmg"
ls -lh "$OUTPUT_DIR/$DMG_NAME.dmg"
