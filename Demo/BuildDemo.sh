#!/usr/bin/env bash
set -euo pipefail

# TohruPhysics Demo Build Verifier
# TohruPhysicsのデモビルド確認スクリプトね。
#
# Verifies that all demo applications compile and link correctly.
# 全てのデモアプリケーションが正しくコンパイル・リンクできるか確認するの。

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/Build"
PASSED=0
FAILED=0

GREEN='\033[32m'
RED='\033[31m'
CYAN='\033[36m'
RESET='\033[0m'

echo -e "${CYAN}=== TohruPhysics Demo Verifier ===${RESET}"
echo

# Configure with demos enabled
echo -e "${CYAN}>>> Configuring...${RESET}"
cmake -B "${BUILD}" -S "${ROOT}" -G Ninja \
	-DTP_BUILD_TESTS=OFF \
	-DTP_BUILD_DEMOS=ON \
	> /dev/null 2>&1
echo

# Build all demos
echo -e "${CYAN}>>> Building demos...${RESET}"
cmake --build "${BUILD}" 2>&1
echo

# Run each demo executable
echo -e "${CYAN}>>> Running demos...${RESET}"
while IFS= read -r -d '' DEMO_EXE; do
	NAME="$(basename "${DEMO_EXE}")"
	echo -e "  ${CYAN}[RUN]${RESET}  ${NAME}"
	if "${DEMO_EXE}" 2>&1; then
		echo -e "  ${GREEN}[ OK]${RESET}  ${NAME}"
		PASSED=$((PASSED + 1))
	else
		echo -e "  ${RED}[FAIL]${RESET} ${NAME}"
		FAILED=$((FAILED + 1))
	fi
	echo
done < <(find "${BUILD}/Demo" -maxdepth 1 -type f -executable -name 'Demo*' -print0 | sort -z)

echo -e "${CYAN}==============================${RESET}"
echo -e "  ${GREEN}Passed:${RESET} ${PASSED}  ${RED}Failed:${RESET} ${FAILED}"
echo -e "${CYAN}==============================${RESET}"

exit ${FAILED}
