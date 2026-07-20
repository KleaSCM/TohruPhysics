#!/usr/bin/env bash
set -euo pipefail

# TohruPhysics Test Runner
# TohruPhysicsのテストランナーね。
#
# Builds and runs every Test* executable under Build/Test/.
# Exits 0 if all pass, 1 if any fail.
# 全てのテストをビルドして実行するの。全部通れば0、一個でも落ちたら1よ。

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/Build"
PASSED=0
FAILED=0

# Colors
GREEN='\033[32m'
RED='\033[31m'
CYAN='\033[36m'
RESET='\033[0m'

echo -e "${CYAN}=== TohruPhysics Test Runner ===${RESET}"
echo

# Step 1: Configure
echo -e "${CYAN}>>> Configuring...${RESET}"
cmake -B "${BUILD}" -S "${ROOT}" -G Ninja -DTP_BUILD_TESTS=ON -DTP_BUILD_DEMOS=OFF > /dev/null 2>&1
echo

# Step 2: Build
echo -e "${CYAN}>>> Building...${RESET}"
cmake --build "${BUILD}" 2>&1
echo

# Step 3: Discover and run test executables
echo -e "${CYAN}>>> Running tests...${RESET}"
while IFS= read -r -d '' TEST_EXE; do
	NAME="$(basename "${TEST_EXE}")"
	echo -e "  ${CYAN}[RUN]${RESET}  ${NAME}"
	if "${TEST_EXE}" 2>&1; then
		echo -e "  ${GREEN}[ OK]${RESET}  ${NAME}"
		PASSED=$((PASSED + 1))
	else
		echo -e "  ${RED}[FAIL]${RESET} ${NAME}"
		FAILED=$((FAILED + 1))
	fi
	echo
done < <(find "${BUILD}/Test" -maxdepth 1 -type f -executable -name 'Test*' -print0 | sort -z)

# Step 4: Report
echo -e "${CYAN}==============================${RESET}"
echo -e "  ${GREEN}Passed:${RESET} ${PASSED}  ${RED}Failed:${RESET} ${FAILED}"
echo -e "${CYAN}==============================${RESET}"

exit ${FAILED}
