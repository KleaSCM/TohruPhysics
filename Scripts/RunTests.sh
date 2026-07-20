#!/usr/bin/env bash
set -euo pipefail

# TohruPhysics test runner
# TohruPhysicsのテストランナーよ。
#
# Builds and runs ALL unit tests. Exits non-zero on any failure.
# 全ての単体テストをビルドして実行するの。失敗したら非ゼロで終了するわ。

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROOT}/build"

echo "=== TohruPhysics Test Runner ==="
echo ""

# Configure
echo ">>> Configuring..."
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
	-DCMAKE_BUILD_TYPE=Debug \
	-DTP_BUILD_TESTS=ON \
	-DTP_BUILD_DEMOS=OFF \
	-G Ninja

echo ""
echo ">>> Building..."
cmake --build "${BUILD_DIR}" --parallel

echo ""
echo ">>> Running tests..."

# Find and run all test executables
# 全てのテスト実行ファイルを探して実行するの。
PASS=0
FAIL=0
while IFS= read -r TEST; do
	NAME="$(basename "${TEST}")"
	echo "  [RUN]  ${NAME}"
	if "${TEST}"; then
		echo "  [ OK]  ${NAME}"
		PASS=$((PASS + 1))
	else
		echo "  [FAIL] ${NAME}"
		FAIL=$((FAIL + 1))
	fi
	echo ""
done < <(find "${BUILD_DIR}" -type f -executable -name 'Test*' | sort)

echo "=============================="
echo "  Passed: ${PASS}  Failed: ${FAIL}"
echo "=============================="

exit ${FAIL}
