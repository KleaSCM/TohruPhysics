#!/usr/bin/env bash
set -euo pipefail

# TohruPhysics Benchmark Runner
# TohruPhysicsのベンチマークランナーね。
#
# Builds and runs all benchmark executables under Bench/.
# 全てのベンチマークをビルドして実行するの。

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/Build"

GREEN='\033[32m'
CYAN='\033[36m'
RESET='\033[0m'

echo -e "${CYAN}=== TohruPhysics Benchmark Runner ===${RESET}"

rm -rf "${BUILD}"
cmake -B "${BUILD}" -S "${ROOT}" -G Ninja \
	-DTP_BUILD_TESTS=OFF \
	-DTP_BUILD_DEMOS=OFF \
	-DTP_BUILD_BENCHMARKS=ON \
	> /dev/null 2>&1

cmake --build "${BUILD}" 2>&1 | tail -3

echo
while IFS= read -r -d '' BENCH_EXE; do
	NAME="$(basename "${BENCH_EXE}")"
	echo -e "${CYAN}>>> ${NAME}${RESET}"
	"${BENCH_EXE}"
	echo
done < <(find "${BUILD}/Bench" -maxdepth 1 -type f -executable -name 'Bench*' -print0 | sort -z)

echo -e "${GREEN}Done.${RESET}"
