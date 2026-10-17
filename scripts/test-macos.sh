#!/bin/bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$project_root"
mkdir -p build/native-tests
xcrun clang++ -std=c++17 -O1 -fobjc-arc -Wall -Wextra -Wno-unused-parameter \
  src/macos/test_macos.mm -framework Cocoa -framework PDFKit -o build/native-tests/test-macos
build/native-tests/test-macos
