#!/bin/bash

PORT=8080
URL="http://localhost:$PORT"
PHASE=$1
OUTPUT_FILE="benchmarks/${PHASE}_results.txt"

if [ -z "$PHASE" ]; then
    echo "Usage: ./wrk_test.sh <phase_name>"
    exit 1
fi

mkdir -p benchmarks
ulimit -n 65535          # ← critical, add this

echo "Building Hyperion server..."
cd build || exit
make -j$(nproc) || exit  # ← parallel build, faster
cd ..

echo "Starting Hyperion server..."
./build/hyperion &
SERVER_PID=$!
trap "kill $SERVER_PID 2>/dev/null; exit" INT TERM EXIT  # ← clean kill

# Wait properly instead of sleep 2
for i in {1..10}; do
    lsof -i :$PORT > /dev/null 2>&1 && break
    sleep 1
done

echo "Running benchmark..."
wrk -t4 -c10000 -d30s --latency $URL | tee $OUTPUT_FILE   # ← --latency added

echo "Benchmark completed. Results in $OUTPUT_FILE"