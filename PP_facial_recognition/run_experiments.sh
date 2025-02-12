#!/bin/bash

# Constants
NUM_ITERS=50
START_RECORDS=10
END_RECORDS=15
DATA_DIR="./data"
EXECUTABLE="./main"
SRC_DIR="./src"

# Ensure data directory exists
mkdir -p $DATA_DIR

# Compile the main program
echo "Compiling main program..."
g++ -o main $SRC_DIR/main.cpp $SRC_DIR/aes_crypt.cpp $SRC_DIR/vector_utils.cpp -Iinclude -lssl -lcrypto -std=c++17
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi
echo "Compilation successful."

# Loop over NUM_RECORDS from 10 to 15
for ((NUM_RECORDS=$START_RECORDS; NUM_RECORDS<=$END_RECORDS; NUM_RECORDS++)); do
    CSV_FILE="$DATA_DIR/plain_run_${NUM_RECORDS}.csv"

    # Write CSV header
    echo "decrypt_time,query_match_time,encrypt_time,result_ctxt_size,total_runtime(ms)" > $CSV_FILE

    echo "Running experiments for NUM_RECORDS=$NUM_RECORDS with NUM_ITERS=$NUM_ITERS..."
    
    # Run NUM_ITERS times
    for ((i=1; i<=NUM_ITERS; i++)); do
        # Measure execution start time
        START_TIME=$(date +%s.%N)

        OUTPUT=$($EXECUTABLE ./test/2_${NUM_RECORDS}.dat)
        EXIT_CODE=$?

        # Measure execution end time
        END_TIME=$(date +%s.%N)

        # Calculate total runtime in milliseconds
        TOTAL_RUNTIME_MS=$(echo "($END_TIME - $START_TIME) * 1000" | bc)

        if [ $EXIT_CODE -ne 0 ]; then
            echo "Error executing main program on iteration $i for NUM_RECORDS=$NUM_RECORDS. Skipping..."
            continue
        fi

        # Convert space-separated output to CSV format and append total runtime
        echo "$OUTPUT" | awk -v runtime="$TOTAL_RUNTIME_MS" '{print $1 "," $2 "," $3 "," $4 "," runtime}' >> $CSV_FILE
        echo "Iteration $i/$NUM_ITERS for NUM_RECORDS=$NUM_RECORDS completed. Total runtime: $TOTAL_RUNTIME_MS ms"
    done

    echo "Completed runs for NUM_RECORDS=$NUM_RECORDS. Results saved in $CSV_FILE."
done

echo "All experiments completed!"
