import os
import subprocess
import argparse

def encrypt_file(input_path, output_path, key):
    # Command to encrypt the file using gramine-sgx-pf-crypt
    command = [
        'gramine-sgx-pf-crypt',
        'encrypt',
        '-w', key,
        '-i', input_path,
        '-o', output_path
    ]
    subprocess.run(command, check=True)

def process_directory(input_dir, output_dir, key):
    for root, dirs, files in os.walk(input_dir):
        # Calculate the relative path
        rel_dir = os.path.relpath(root, input_dir)
        # Create the corresponding directory in the output path
        target_dir = os.path.join(output_dir, rel_dir)
        os.makedirs(target_dir, exist_ok=True)
        for file in files:
            input_file = os.path.join(root, file)
            output_file = os.path.join(target_dir, file)
            print(f'Encrypting {input_file} to {output_file}')
            encrypt_file(input_file, output_file, key)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Encrypt files under a given folder for use with Gramine.')
    parser.add_argument('input_dir', help='Path to the input directory containing files to encrypt.')
    parser.add_argument('output_dir', help='Path to the output directory where encrypted files will be stored.')
    parser.add_argument('key', help='32-byte hex encryption key (64 hex characters).')

    args = parser.parse_args()

    process_directory(args.input_dir, args.output_dir, args.key)
