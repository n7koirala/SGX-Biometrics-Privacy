#!/bin/bash

# Set the path for binaries and source files
BIN_DIR="./bin"
SRC_DIR="./src"

# Ensure the bin directory exists
if [ ! -d "$BIN_DIR" ]; then
    echo "Creating bin directory..."
    mkdir -p $BIN_DIR
fi

# Set default values (2^10 to 2^20) if not provided
LOW_EXP=${1:-10}   # First argument or default to 10
HIGH_EXP=${2:-20}  # Second argument or default to 20

# Compile necessary C++ files
echo "Compiling programs..."
g++ -o $BIN_DIR/create_sender_db $SRC_DIR/create_sender_db.cpp -std=c++17
g++ -o $BIN_DIR/encrypt_receiver $SRC_DIR/encrypt_receiver.cpp $SRC_DIR/aes_crypt.cpp -Iinclude -lssl -lcrypto -std=c++17
g++ -o $BIN_DIR/main $SRC_DIR/main.cpp $SRC_DIR/aes_crypt.cpp -Iinclude -lssl -lcrypto -std=c++17

# Loop over different sizes (2^LOW_EXP to 2^HIGH_EXP)
for ((exp=LOW_EXP; exp<=HIGH_EXP; exp++)); do
    num_records=$((2**exp))
    echo "Running with $num_records records..."

    # Step 1: Create sender database with `num_records`
    echo "Generating sender database..."
    $BIN_DIR/create_sender_db $num_records

    # Step 2: Encrypt the receiver query
    echo "Encrypting receiver query..."
    $BIN_DIR/encrypt_receiver

    # Step 3: Run the main program for intersection
    echo "Running main program..."
    $BIN_DIR/main 1   # Pass '1' as number of senders since it's a single sender file

    echo "------------------------------------------------"
done

echo "Experiment completed!"
