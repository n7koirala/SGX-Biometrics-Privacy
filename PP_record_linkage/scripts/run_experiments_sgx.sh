#!/bin/bash

# Define output file
OUTPUT_FILE="sgx_run_results.txt"
NUM_RUNS=25
SENDERS_LIST=(8 16 32 64 128 256 512 1024)

# Clear output file before starting
echo "SGX Experiment Results" > $OUTPUT_FILE
echo "=======================" >> $OUTPUT_FILE

# Function to calculate statistics
calculate_statistics() {
    local numbers=("$@")
    local count=${#numbers[@]}
    
    if [[ $count -eq 0 ]]; then
        echo "No valid data collected!"
        return
    fi

    # Compute sum and find min/max
    local sum=0
    local min=${numbers[0]}
    local max=${numbers[0]}
    
    for num in "${numbers[@]}"; do
        sum=$(echo "$sum + $num" | bc)
        (( $(echo "$num < $min" | bc -l) )) && min=$num
        (( $(echo "$num > $max" | bc -l) )) && max=$num
    done
    
    # Compute mean
    local mean=$(echo "scale=6; $sum / $count" | bc)

    # Compute standard deviation
    local sum_sq=0
    for num in "${numbers[@]}"; do
        sum_sq=$(echo "$sum_sq + ($num - $mean)^2" | bc)
    done
    local stddev=$(echo "scale=6; sqrt($sum_sq / $count)" | bc)

    # Compute median
    sorted_numbers=($(printf '%s\n' "${numbers[@]}" | sort -n))
    local mid=$((count / 2))
    local median=${sorted_numbers[$mid]}
    if (( count % 2 == 0 )); then
        median=$(echo "scale=6; (${sorted_numbers[$mid - 1]} + ${sorted_numbers[$mid]}) / 2" | bc)
    fi

    echo "Min: $min ms, Max: $max ms, Avg: $mean ms, Median: $median ms, Std Dev: $stddev ms"
}

# Loop through each Num_Senders value
for NUM_SENDERS in "${SENDERS_LIST[@]}"; do
    echo "Running tests for Num_Senders = $NUM_SENDERS..." | tee -a $OUTPUT_FILE
    echo "-------------------------------------------------" >> $OUTPUT_FILE

    # Clean and build
    make clean >> $OUTPUT_FILE 2>&1
    make SGX=1 NUM_SENDERS=$NUM_SENDERS >> $OUTPUT_FILE 2>&1

    INTERSECTION_TIMES=()
    TOTAL_TIMES=()

    for ((i=1; i<=NUM_RUNS; i++)); do
        START_TIME=$(date +%s.%N)
        OUTPUT=$(gramine-sgx ./build/ReceiverSender 2>&1)
        END_TIME=$(date +%s.%N)

        # Extract only the first numeric value from the output (intersection time)
        INTERSECTION_TIME=$(echo "$OUTPUT" | grep -oE '^[0-9]+\.[0-9]+' | head -n 1)

        # Validate intersection time
        if [[ -z "$INTERSECTION_TIME" || ! "$INTERSECTION_TIME" =~ ^[0-9]+\.[0-9]+$ ]]; then
            echo "Warning: Skipping invalid output: $OUTPUT" | tee -a $OUTPUT_FILE
            continue
        fi

        # Validate start and end time before using bc
        if [[ ! $START_TIME =~ ^[0-9.]+$ ]] || [[ ! $END_TIME =~ ^[0-9.]+$ ]]; then
            echo "Error: START_TIME or END_TIME is invalid!" | tee -a $OUTPUT_FILE
            continue
        fi

        # Calculate total execution time in milliseconds
        TOTAL_TIME=$(echo "($END_TIME - $START_TIME) * 1000" | bc 2>/dev/null)

        # Validate total execution time
        if [[ ! $TOTAL_TIME =~ ^[0-9.]+$ ]]; then
            echo "Error: TOTAL_TIME calculation failed!" | tee -a $OUTPUT_FILE
            continue
        fi

        # Store values
        INTERSECTION_TIMES+=($INTERSECTION_TIME)
        TOTAL_TIMES+=($TOTAL_TIME)

        echo "$INTERSECTION_TIME $TOTAL_TIME" | tee -a $OUTPUT_FILE
    done

    # Compute statistics and append to output file
    if [ ${#INTERSECTION_TIMES[@]} -gt 0 ]; then
        echo "Statistics for Num_Senders = $NUM_SENDERS" >> $OUTPUT_FILE
        echo "Intersection Time Statistics:" >> $OUTPUT_FILE
        STATS=$(calculate_statistics "${INTERSECTION_TIMES[@]}")
        echo "$STATS" | tee -a $OUTPUT_FILE
    else
        echo "No valid intersection data collected for Num_Senders = $NUM_SENDERS" | tee -a $OUTPUT_FILE
    fi

    if [ ${#TOTAL_TIMES[@]} -gt 0 ]; then
        echo "Total Execution Time Statistics:" >> $OUTPUT_FILE
        STATS=$(calculate_statistics "${TOTAL_TIMES[@]}")
        echo "$STATS" | tee -a $OUTPUT_FILE
        echo "" >> $OUTPUT_FILE
    else
        echo "No valid total execution data collected for Num_Senders = $NUM_SENDERS" | tee -a $OUTPUT_FILE
    fi
done

echo "Experiment completed. Results saved in $OUTPUT_FILE."
