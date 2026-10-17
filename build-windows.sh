#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$project_root"
compiler="${CXX:-g++}"
resource_compiler="${WINDRES:-windres}"
mkdir -p build/windows dist
(
  cd src
  "$resource_compiler" --codepage=65001 app.rc -O coff -o ../build/windows/app.res
)
"$compiler" src/main.cpp build/windows/app.res -std=c++17 -Os -s \
  -static -static-libgcc -static-libstdc++ -municode -mwindows \
  -o dist/eScorerDOCs.exe \
  -lcomctl32 -lshell32 -lole32 -ladvapi32 -lgdi32 -luser32 \
  -lruntimeobject -lshcore -lshlwapi -lwindowscodecs -luuid -lcomdlg32
echo 'Gotowe: dist/eScorerDOCs.exe'
