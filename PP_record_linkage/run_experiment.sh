#!/bin/bash

# Usage: ./run_experiment.sh

# List of NUM_RECORDS values (2^10 to 2^20)
NUM_RECORDS_LIST=(1024 2048 4096 8192 16384 32768 65536 131072 262144 262144 524288 1048576)
NUM_ITERS=50

# Directories
BIN_DIR="bin"
SRC_DIR="src"
DATA_DIR="data"
SENDER_DB="sender_db"
RESULTS_RECEIVER="$DATA_DIR/receiver_results.csv"
STATS_RECEIVER="$DATA_DIR/receiver_statistics.csv"
STATS_SENDER="$DATA_DIR/sender_statistics.csv"

# Ensure necessary directories exist
mkdir -p $BIN_DIR $DATA_DIR $SENDER_DB

# Clear previous receiver results
echo "hash_value,ciphertext_size,encryption_time,total_runtime" > $RESULTS_RECEIVER

# Compile create_sender_db.cpp
echo "Compiling create_sender_db.cpp..."
g++ -o $BIN_DIR/create_sender_db $SRC_DIR/create_sender_db.cpp -Iinclude -std=c++17
if [ $? -ne 0 ]; then
    echo "Compilation failed for create_sender_db.cpp"
    exit 1
fi

# Compile encrypt_receiver.cpp
echo "Compiling encrypt_receiver.cpp..."
g++ -o $BIN_DIR/encrypt_receiver $SRC_DIR/encrypt_receiver.cpp $SRC_DIR/aes_crypt.cpp -Iinclude -lssl -lcrypto -std=c++17
if [ $? -ne 0 ]; then
    echo "Compilation failed for encrypt_receiver.cpp"
    exit 1
fi

# Iterate over each NUM_RECORDS value
for NUM_RECORDS in "${NUM_RECORDS_LIST[@]}"; do
    echo "Processing NUM_RECORDS=$NUM_RECORDS"

    # Clear sender_set.txt
    rm -f $SENDER_DB/sender_set.txt

    # Run create_sender_db to generate data
    echo "Generating $NUM_RECORDS records..."
    $BIN_DIR/create_sender_db $NUM_RECORDS
    if [ $? -ne 0 ]; then
        echo "Error running create_sender_db"
        exit 1
    fi

    # Measure encryption runtime
    ENC_START=$(date +%s.%N)
    ENC_OUTPUT=$($BIN_DIR/encrypt_receiver)
    ENC_END=$(date +%s.%N)
    ENC_RUNTIME=$(echo "$ENC_END - $ENC_START" | bc)

    if [ $? -ne 0 ]; then
        echo "Error running encrypt_receiver"
        exit 1
    fi

    # Extract values and save to receiver_results.csv
    echo "$ENC_OUTPUT" | awk -v runtime="$ENC_RUNTIME" '{print $1 "," $2 "," $3 "," runtime}' >> $RESULTS_RECEIVER

    # Compile main.cpp only ONCE per NUM_RECORDS
    echo "Compiling main.cpp..."
    g++ -o $BIN_DIR/main $SRC_DIR/main.cpp $SRC_DIR/aes_crypt.cpp -Iinclude -lssl -lcrypto -std=c++17
    if [ $? -ne 0 ]; then
        echo "Compilation failed for main.cpp"
        exit 1
    fi

    # Create a unique CSV file for each NUM_RECORDS
    RESULTS_SENDER="$DATA_DIR/sender_results_${NUM_RECORDS}.csv"
    echo "decrypted_hash,decryption_time,intersection_time,intersection_result,encryption_time,ciphertext_size,total_runtime" > $RESULTS_SENDER

    # Run main.cpp multiple times
    for ((i=1; i<=NUM_ITERS; i++)); do
        echo "Running main.cpp (Iteration $i for NUM_RECORDS=$NUM_RECORDS)"

        # Measure total runtime for main.cpp
        MAIN_START=$(date +%s.%N)
        MAIN_OUTPUT=$($BIN_DIR/main)
        MAIN_END=$(date +%s.%N)
        MAIN_RUNTIME=$(echo "$MAIN_END - $MAIN_START" | bc)

        if [ $? -ne 0 ]; then
            echo "Error running main.cpp"
            exit 1
        fi

        # Extract values and save to sender_results_<NUM_RECORDS>.csv
        echo "$MAIN_OUTPUT" | awk -v runtime="$MAIN_RUNTIME" '{print $1 "," $2 "," $3 "," $4 "," $5 "," $6 "," runtime}' >> $RESULTS_SENDER
    done
done

# Compute statistics using Python
# echo "Computing statistics..."
# python3 <<EOF
# import pandas as pd
# import glob

# # Load receiver results
# receiver_df = pd.read_csv("$RESULTS_RECEIVER")
# receiver_stats = receiver_df['total_runtime'].describe()
# receiver_stats.to_csv("$STATS_RECEIVER", header=["Receiver Total Runtime Stats"])

# # Compute sender statistics for all NUM_RECORDS values
# sender_stat_list = []
# for file in glob.glob("$DATA_DIR/sender_results_*.csv"):
#     df = pd.read_csv(file)
#     num_records = file.split('_')[-1].split('.')[0]
#     sender_runtime_stats = df['total_runtime'].describe()
#     sender_intersection_stats = df['intersection_time'].describe()
#     sender_stat_list.append((num_records, sender_runtime_stats, sender_intersection_stats))

# # Write sender statistics to file
# with open("$STATS_SENDER", "w") as f:
#     for num_records, runtime_stats, intersection_stats in sender_stat_list:
#         f.write(f"Sender Stats for NUM_RECORDS={num_records}\n")
#         f.write(runtime_stats.to_csv())
#         f.write("\nSender Intersection Time Stats\n")
#         f.write(intersection_stats.to_csv())
#         f.write("\n")

# print("Statistics saved to receiver_statistics.csv and sender_statistics.csv")
# EOF

echo "Experiment completed successfully."
