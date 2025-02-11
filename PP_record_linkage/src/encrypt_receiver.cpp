#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>  // Required for memcpy
#include "aes_crypt.h"
#include "file_paths.h"


// Function to create directories if they don't exist
void create_directory(const std::string& path) {
    mkdir(path.c_str(), 0777);
}

// Function to compute 32-bit hash using SHA-256 and extract first 4 bytes
uint32_t hash_data(const std::string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)data.c_str(), data.size(), hash);
    return *(uint32_t*)hash;  // Take first 4 bytes
}


// Function to write encrypted data to file
void write_encrypted_file(const std::vector<unsigned char>& ciphertext, 
                          const std::vector<unsigned char>& iv) {
    std::ofstream outfile(ENCRYPTED_FILE, std::ios::binary);
    outfile.write((char*)iv.data(), IV_SIZE);  // Store IV at the beginning
    outfile.write((char*)ciphertext.data(), ciphertext.size());
    outfile.close();
}

// Function to generate dummy receiver watchlist and encrypt it
void encrypt_receiver_query() {
    create_directory("receiver_query");
    create_directory("enc_receiver_query");

    // Dummy data: Simulated watchlist entry 
    std::string person_info = "{Name: John Doe, DOB: 1990-05-23, Passport #: C03004786, Signature: 42424242424242}";
    
    // Compute 32-bit hash
    uint32_t hashed_value = hash_data(person_info);

    // Save hashed data to receiver file
    std::ofstream receiver_file(RECEIVER_FILE);
    receiver_file << hashed_value << "\n";
    receiver_file.close();

    // Convert hash to bytes
    std::vector<unsigned char> plaintext(sizeof(uint32_t));
    std::memcpy(plaintext.data(), &hashed_value, sizeof(uint32_t));

    // Generate AES key and IV
    std::vector<unsigned char> key = generate_aes_key();
    std::vector<unsigned char> iv = generate_iv();

    // Store the AES key and IV
    write_aes_key_iv(AES_KEY_FILE, AES_IV_FILE, key, iv);

    // Measure encryption time
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> ciphertext = aes_encrypt_ctr(plaintext, key, iv);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> encryption_time = end_time - start_time;

    // Save encrypted data
    write_encrypted_file(ciphertext, iv);

    // Print results
    /*
    std::cout << "Original Data: " << person_info << std::endl;
    std::cout << "32-bit Hashed Value: " << hashed_value << std::endl;
    std::cout << "AES Key and IV have been saved in aes_keys/ directory.\n";
    std::cout << "Ciphertext Size: " << ciphertext.size() << " bytes" << std::endl;
    std::cout << "Encryption Time: " << encryption_time.count() << " ms" << std::endl;
    */
    std::cout << hashed_value << " ";
    std::cout << ciphertext.size() << " ";
    std::cout << encryption_time.count() << std::endl;
}

int main() {
    encrypt_receiver_query();
    return 0;
}
