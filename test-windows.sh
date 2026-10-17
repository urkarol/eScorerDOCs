#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$project_root"
case "$(uname -s)" in MINGW*|MSYS*) ;; *) echo 'Ten test wymaga Windows/MSYS2.' >&2; exit 1;; esac
compiler="${CXX:-g++}"
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/eScorerDOCs-windows.XXXXXXXX")"
trap 'rm -rf "$test_dir"' EXIT
native_dir="$(cygpath -w "$test_dir")"
# Compile with assertions enabled; -DNDEBUG would disable real test operations.
"$compiler" -std=c++17 -O2 -static -municode "$project_root/src/test_preview.cpp" \
  -o "$test_dir/test_preview.exe" \
  -lruntimeobject -lshcore -lshlwapi -lwindowscodecs -luuid -lcomdlg32 -lgdi32 -luser32 -lole32
cd "$test_dir"
./test_preview.exe
"$project_root/dist/eScorerDOCs.exe" --self-test "$native_dir/generated"
echo 'Test renderowania PDF, usuwania plików tymczasowych i zapisu przez EXE zakończony poprawnie.'
