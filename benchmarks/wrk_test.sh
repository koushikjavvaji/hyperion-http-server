#!/bin/bash

PORT=8080
URL="http://localhost:$PORT"
PHASE=$1
OUTPUT_FILE="benchmarks/${PHASE}_results.txt"

echo "Building Hyperion server..."
cd build || exit
make || exit
cd ..

echo "Starting Hyperion server..."

./build/hyperion &
SERVER_PID=$!

sleep 2

# check if server actually started
if ! lsof -i :$PORT > /dev/null ; then
    echo "Server failed to start on port $PORT"
    exit 1
fi

echo "Running benchmark..."

wrk -t4 -c5000 -d10s $URL > $OUTPUT_FILE

echo "Stopping server..."

kill $SERVER_PID 2>/dev/null

echo "Benchmark completed!"
echo "Results saved to $OUTPUT_FILE"
