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

BUILD_CMD="cmake --build --preset debug"
EXEC="./bin/yagl -c -r ./run"

if [[ "$key" == "r" ]]; then
  echo "Running cmake configuration..."
  cmake --preset debug -G Ninja
  echo "Building..."
  $BUILD_CMD
  echo "Running..."
  $EXEC
elif [[ "$key" == "g" ]]; then
  echo "Building..."
  $BUILD_CMD
  echo "Running gdb..."
  gdb --args "$EXEC"
elif [[ "$key" == "p" ]]; then
  echo "Building..."
  $BUILD_CMD
  echo "Profiling with perf and generating per-thread Inferno flame graph..."

  # Run perf record (sampling all threads)
  perf record -F 1000 -g -- "$EXEC"

  # Generate per-thread flame graph
  perf script | inferno-collapse-perf --tid | inferno-flamegraph >flamegraph-per-thread.svg

  echo "Per-thread Inferno flame graph generated: flamegraph-per-thread.svg"
  echo "Open it in a browser to inspect thread workloads and hotspots."

  echo "Optional: Use thread naming in your game (pthread_setname_np) to make the graph more readable."
elif [[ "$key" == "b" ]]; then
  echo "Building..."
  $BUILD_CMD
else
  echo "Building..."
  $BUILD_CMD
  echo "Running..."
  $EXEC
fi
