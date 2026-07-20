#!/usr/bin/env bash
set -euo pipefail

# TohruPhysics Sanitizer Runner
# TohruPhysicsのサニタイザーランナーね。
#
# Builds and runs all tests with UndefinedBehaviorSanitizer and AddressSanitizer.
# Exits 0 only if all tests pass under both sanitizers.
# UBSanとASanの両方で全テストをビルドして実行するの。両方通ったときだけ0で終了よ。

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/Build"
PASS=0
FAIL=0

GREEN='\033[32m'
RED='\033[31m'
CYAN='\033[36m'
RESET='\033[0m'

run_sanitizer() {
	local NAME="$1"
	local UBSAN="$2"
	local ASAN="$3"

	echo -e "${CYAN}========================================${RESET}"
	echo -e "${CYAN}  Sanitizer: ${NAME}${RESET}"
	echo -e "${CYAN}========================================${RESET}"

	rm -rf "${BUILD}"
	cmake -B "${BUILD}" -S "${ROOT}" -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DTP_BUILD_TESTS=ON \
		-DTP_BUILD_DEMOS=OFF \
		-DTP_BUILD_BENCHMARKS=OFF \
		-DTP_USE_UBSAN="${UBSAN}" \
		-DTP_USE_ASAN="${ASAN}" \
		> /dev/null 2>&1

	cmake --build "${BUILD}" 2>&1 | tail -5

	local SUITE_PASS=0
	local SUITE_FAIL=0
	while IFS= read -r -d '' TEST_EXE; do
		local TNAME="$(basename "${TEST_EXE}")"
		echo -e "  ${CYAN}[RUN]${RESET}  ${TNAME}"
		if "${TEST_EXE}" 2>&1; then
			echo -e "  ${GREEN}[ OK]${RESET}  ${TNAME}"
			SUITE_PASS=$((SUITE_PASS + 1))
		else
			echo -e "  ${RED}[FAIL]${RESET} ${TNAME}"
			SUITE_FAIL=$((SUITE_FAIL + 1))
		fi
	done < <(find "${BUILD}/Test" -maxdepth 1 -type f -executable -name 'Test*' -print0 | sort -z)

	echo
	if [ "${SUITE_FAIL}" -eq 0 ]; then
		echo -e "${GREEN}  ${NAME}: all ${SUITE_PASS} passed${RESET}"
		PASS=$((PASS + 1))
	else
		echo -e "${RED}  ${NAME}: ${SUITE_FAIL} failed${RESET}"
		FAIL=$((FAIL + 1))
	fi
	echo
}

run_sanitizer "UndefinedBehaviorSanitizer" "ON" "OFF"
run_sanitizer "AddressSanitizer" "OFF" "ON"

echo -e "${CYAN}========================================${RESET}"
echo -e "${CYAN}  Summary${RESET}"
echo -e "${CYAN}========================================${RESET}"
echo -e "  ${GREEN}Passed:${RESET} ${PASS}  ${RED}Failed:${RESET} ${FAIL}"

exit ${FAIL}
