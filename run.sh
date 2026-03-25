#!/bin/bash
set -e

DIR="$(dirname "$(realpath "$0")")"
cd "$DIR"

echo "Options:"
echo "  r → rerun cmake"
echo "  b → only build"
echo "  g → run gdb"
echo "  p → run with perf + per-thread Inferno flame graph"
echo "  Enter → build and run normally"
read -r -n1 key
echo

BUILD_CMD=(cmake --build --preset debug)
ARGS=(-c -r ./run)
EXEC=("./bin/yagl")

if [[ "$key" == "r" ]]; then
  echo "Running cmake configuration..."
  cmake --preset debug -G Ninja
  echo "Building..."
  "${BUILD_CMD[@]}"
  echo "Running..."
  "${EXEC[@]}" "${ARGS[@]}"
elif [[ "$key" == "g" ]]; then
  echo "Building..."
  "${BUILD_CMD[@]}"
  echo "Running gdb..."
  gdb --args "${EXEC[@]}" "${ARGS[@]}"
elif [[ "$key" == "p" ]]; then
  echo "Building..."
  "${BUILD_CMD[@]}"
  echo "Profiling with perf and generating per-thread Inferno flame graph..."
  perf record -F 1000 -g -- "${EXEC[@]}" "${ARGS[@]}"
  perf script | inferno-collapse-perf --tid | inferno-flamegraph >flamegraph-per-thread.svg
  echo "Per-thread Inferno flame graph generated: flamegraph-per-thread.svg"
elif [[ "$key" == "b" ]]; then
  echo "Building..."
  "${BUILD_CMD[@]}"
else
  echo "Building..."
  "${BUILD_CMD[@]}"
  echo "Running..."
  "${EXEC[@]}" "${ARGS[@]}"
fi
