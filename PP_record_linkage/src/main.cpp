#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>
#include <iomanip>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <cstring> 
#include "aes_crypt.h"
#include "file_paths.h"

// Function to read and decrypt the receiver query
bool decrypt_receiver_query(uint32_t& receiver_value) {
    // Load AES key and IV
    std::vector<unsigned char> key, iv;
    if (!read_aes_key_iv(AES_KEY_FILE, AES_IV_FILE, key, iv)) {
        return false;
    }

    // Read the encrypted file
    std::ifstream enc_file(ENC_RECEIVER_FILE, std::ios::binary);
    if (!enc_file.is_open()) {
        std::cerr << "Error opening encrypted receiver file: " << ENC_RECEIVER_FILE << std::endl;
        return false;
    }

    // Read IV (first 16 bytes)
    enc_file.read((char*)iv.data(), IV_SIZE);

    // Read ciphertext
    std::vector<unsigned char> ciphertext((std::istreambuf_iterator<char>(enc_file)), std::istreambuf_iterator<char>());
    enc_file.close();

    if (ciphertext.empty()) {
        std::cerr << "Error: Encrypted file is empty!" << std::endl;
        return false;
    }

    // Measure decryption time
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> decrypted_data = aes_decrypt_ctr(ciphertext, key, iv);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> decryption_time = end_time - start_time;

    if (decrypted_data.size() != sizeof(uint32_t)) {
        std::cerr << "Error: Decrypted data size mismatch!" << std::endl;
        return false;
    }

    // Copy decrypted value into receiver_value
    std::memcpy(&receiver_value, decrypted_data.data(), sizeof(uint32_t));

    // Print results
    std::cout << receiver_value << " ";
    std::cout << decryption_time.count() << " ";

    return true;
}

// Function to encrypt and store the boolean result
bool encrypt_result(bool found) {
    // Load AES key and IV
    std::vector<unsigned char> key, iv;
    if (!read_aes_key_iv(AES_KEY_FILE, AES_IV_FILE, key, iv)) {
        return false;
    }

    // Convert boolean to uint8_t (1 byte)
    uint8_t result_value = found ? 1 : 0;
    //std::cout << "Result Value: " << static_cast<int>(result_value) << std::endl;

    std::vector<unsigned char> plaintext(1, result_value); // 1 byte to store bool

    // Measure encryption time
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> ciphertext = aes_encrypt_ctr(plaintext, key, iv);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> encryption_time = end_time - start_time;

    // **Ensure the "result" directory exists**
    struct stat st;
    if (stat("result", &st) != 0) {
        if (mkdir("result", 0777) != 0) {
            std::cerr << "Error creating directory: result\n";
            return false;
        }
    }

    // Write encrypted result to file
    std::ofstream enc_file(ENC_RESULT_FILE, std::ios::binary);
    if (!enc_file.is_open()) {
        std::cerr << "Error opening result file: " << ENC_RESULT_FILE << std::endl;
        return false;
    }
    enc_file.write((char*)ciphertext.data(), ciphertext.size());
    enc_file.close();

    // Print encryption details
    /*
    std::cout << "Encrypted Boolean Result: " << static_cast<int>(result_value) << std::endl;
    std::cout << "Encryption Time: " << encryption_time.count() << " ms" << std::endl;
    std::cout << "Ciphertext Size: " << ciphertext.size() << " bytes\n";
    */

    std::cout <<  encryption_time.count() << " ";
    std::cout << ciphertext.size() << std::endl;

    return true;
}

int main() {
    std::unordered_set<uint32_t> receiver_set;
    uint32_t receiver_value;

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::duration<double> elapsed;

    start = std::chrono::steady_clock::now();

    // Decrypt the receiver query
    if (!decrypt_receiver_query(receiver_value)) {
        return 1;
    }

    // Insert decrypted value into receiver set
    receiver_set.insert(receiver_value);

    bool found = false;

    // Read from the single sender file
    std::ifstream sender_file(SENDER_FILE);
    if (!sender_file.is_open()) {
        std::cerr << "Error opening sender file: " << SENDER_FILE << std::endl;
        return 1;
    }

    std::unordered_set<uint32_t> sender_set;
    uint32_t value;

    // Load the sender set
    while (sender_file >> value) {
        sender_set.insert(value);
    }
    sender_file.close();

    // Perform set intersection with receiver set
    if (sender_set.find(receiver_value) != sender_set.end()) {
        found = true;
    }

    stop = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << elapsed.count();

    if (found) {
        std::cout << " 1" << " ";
    } else {
        std::cout << " 0" << " ";
    }

    // Encrypt and save the result
    if (!encrypt_result(found)) {
        std::cerr << "Error encrypting result.\n";
        return 1;
    }

    return 0;
}
