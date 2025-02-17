#!/usr/bin/env python3
import csv
import glob

# Get all sender_results_*.csv files in the ./data directory
file_list = glob.glob('./data/sgx/sender_results_*.csv')

for file_name in file_list:
    print(f"Processing {file_name}...")
    # Read in the CSV data
    with open(file_name, mode='r', newline='') as infile:
        reader = csv.reader(infile)
        rows = list(reader)

    # Assume the first row is the header
    header = rows[0]
    
    # Process all data rows (skip header)
    for i in range(1, len(rows)):
        try:
            # Convert the total_runtime value to float and multiply by 1000
            total_runtime = float(rows[i][-1])
            new_runtime = total_runtime * 1000
            # Convert back to string (you can format if needed)
            rows[i][-1] = str(new_runtime)
        except ValueError:
            # If conversion fails, print a warning and skip the row
            print(f"Warning: could not convert value {rows[i][-1]} in file {file_name}")
    
    # Write the updated data back to the same file
    with open(file_name, mode='w', newline='') as outfile:
        writer = csv.writer(outfile)
        writer.writerows(rows)
