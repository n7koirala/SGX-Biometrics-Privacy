#!/bin/bash

# Define paths
SRC_DIR="./src"
INCLUDE_DIR="./include"
OUTPUT_DIR="./data/receiver"
OUTPUT_FILE="$OUTPUT_DIR/receiver_encrypt_results.csv"

# Ensure the output directory exists
mkdir -p "$OUTPUT_DIR"

# Compile the receiver_encrypt program
echo "Compiling receiver_encrypt..."
g++ -o receiver_encrypt "$SRC_DIR/receiver_encrypt.cpp" "$SRC_DIR/aes_crypt.cpp" -I"$INCLUDE_DIR" -lssl -lcrypto -std=c++17

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

echo "Compilation successful."

# Run the program NUM_ITERS times
NUM_ITERS=50

# Write CSV header
echo "ciphertext_size,encrypt_time,total_execution_time" > "$OUTPUT_FILE"

echo "Running receiver_encrypt $NUM_ITERS times..."
for ((i=1; i<=NUM_ITERS; i++))
do
    # Measure execution time
    START_TIME=$(date +%s%3N)  # Get start time in milliseconds
    OUTPUT=$(./receiver_encrypt)
    END_TIME=$(date +%s%3N)  # Get end time in milliseconds
    
    # Calculate total execution time in ms
    TOTAL_TIME=$((END_TIME - START_TIME))

    # Extract ciphertext_size and encrypt_time
    CIPHERTEXT_SIZE=$(echo "$OUTPUT" | awk '{print $1}')
    ENCRYPT_TIME=$(echo "$OUTPUT" | awk '{print $2}')

    # Check if the values are valid numbers
    if [[ ! $CIPHERTEXT_SIZE =~ ^[0-9]+$ ]] || [[ ! $ENCRYPT_TIME =~ ^[0-9]*\.[0-9]+$ ]]; then
        echo "Unexpected output format: '$OUTPUT'. Skipping this run."
        continue
    fi

    # Append to CSV file
    echo "$CIPHERTEXT_SIZE,$ENCRYPT_TIME,$TOTAL_TIME" >> "$OUTPUT_FILE"

    echo "Run $i: $CIPHERTEXT_SIZE, $ENCRYPT_TIME, $TOTAL_TIME ms"
done

echo "All runs completed. Results saved in $OUTPUT_FILE"
