#!/usr/bin/env bash
set -euo pipefail

# Delegates to the canonical test runner in Test/.
# Test/にある正規のテストランナーに委譲するの。

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
exec "${ROOT}/Test/RunTests.sh" "$@"
