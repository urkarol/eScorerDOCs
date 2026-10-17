#!/bin/bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$project_root"
if [[ "$(uname -s)" != Darwin ]]; then
  echo 'Budowanie aplikacji wymaga macOS. Użyj GitHub Actions lub komputera Mac.' >&2
  exit 1
fi
architecture="${1:-arm64}"
case "$architecture" in arm64|x86_64) ;; *) echo 'Dozwolone architektury: arm64, x86_64' >&2; exit 1;; esac
build_dir="$project_root/build/macos-$architecture"
app_dir="$build_dir/eScorerDOCs.app"
mkdir -p "$app_dir/Contents/MacOS" "$app_dir/Contents/Resources" "$build_dir/AppIcon.iconset" "$project_root/dist"
xcrun clang++ -std=c++17 -O2 -fobjc-arc -Wall -Wextra -Wno-unused-parameter \
  -arch "$architecture" -mmacosx-version-min=13.0 \
  src/macos/main.mm -framework Cocoa -framework PDFKit \
  -o "$app_dir/Contents/MacOS/eScorerDOCs"
cp src/macos/Info.plist "$app_dir/Contents/Info.plist"
cp DejaVu-LICENSE.txt "$app_dir/Contents/Resources/DejaVu-LICENSE.txt"
xcrun clang++ -std=c++17 -O2 -fobjc-arc src/macos/icon.mm -framework Cocoa -o "$build_dir/make-icon"
"$build_dir/make-icon" "$build_dir"
for size in 16 32 128 256 512; do
  cp "$build_dir/render-$size.png" "$build_dir/AppIcon.iconset/icon_${size}x${size}.png"
  double=$((size * 2))
  cp "$build_dir/render-$double.png" "$build_dir/AppIcon.iconset/icon_${size}x${size}@2x.png"
done
iconutil -c icns "$build_dir/AppIcon.iconset" -o "$app_dir/Contents/Resources/AppIcon.icns"
plutil -lint "$app_dir/Contents/Info.plist"
# Ad-hoc signature is required for Apple Silicon. Developer ID/notarization is a separate distribution step.
codesign --force --sign - "$app_dir"
codesign --verify --deep --strict --verbose=2 "$app_dir"
ditto -c -k --sequesterRsrc --keepParent "$app_dir" "$project_root/dist/eScorerDOCs-macOS-$architecture.zip"
echo "Gotowe: dist/eScorerDOCs-macOS-$architecture.zip"
