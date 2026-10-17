#!/usr/bin/env bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$project_root"
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/eScorerDOCs-tests.XXXXXXXX")"
trap 'rm -rf "$test_dir"' EXIT
compiler="${CXX:-c++}"
for test in core roster naming lineup; do
  "$compiler" -std=c++17 -O2 -Wall -Wextra "$project_root/src/test_$test.cpp" -o "$test_dir/test_$test"
done
# Run tests in an isolated directory; repeated runs cannot collide with existing PDFs.
cd "$test_dir"
./test_core "$test_dir/blank.pdf"
./test_roster
./test_naming
./test_lineup "$test_dir/lineups"
echo 'Testy wspólnej logiki zakończone poprawnie.'
