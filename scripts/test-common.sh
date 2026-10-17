#!/bin/bash
set -euo pipefail
project_root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$project_root"
test_dir="$(mktemp -d "${TMPDIR:-/tmp}/eScorerDOCs-tests.XXXXXXXX")"
trap 'rm -rf "$test_dir"' EXIT
compiler="${CXX:-c++}"
for test in core roster naming lineup; do
  "$compiler" -std=c++17 -O2 -Wall -Wextra "src/test_$test.cpp" -o "$test_dir/test_$test"
done
"$test_dir/test_core" "$test_dir/blank.pdf"
"$test_dir/test_roster"
"$test_dir/test_naming" "$test_dir/naming"
"$test_dir/test_lineup" "$test_dir/lineups"
echo 'Testy wspólnej logiki zakończone poprawnie.'
