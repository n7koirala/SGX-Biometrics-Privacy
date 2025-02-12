#include "config.h"


// Function to create directories if they don't exist
void create_directory(const std::string& path) {
    mkdir(path.c_str(), 0777);
}

// Function to write encrypted data to file
void write_encrypted_file(const std::vector<unsigned char>& ciphertext, 
                          const std::vector<unsigned char>& iv) {
    std::ofstream outfile(ENCRYPTED_FILE, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not open file " << ENCRYPTED_FILE << " for writing." << std::endl;
        return;
    }
    // Write IV first
    outfile.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE);
    // Then write the ciphertext
    outfile.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
    outfile.close();
}

// Function to read plaintext vector from query file
std::vector<unsigned char> read_plaintext_vector() {
    std::ifstream infile(QUERY_FILE_PATH);
    if (!infile.is_open()) {
        throw std::runtime_error("Error: Unable to open query file: " + std::string(QUERY_FILE_PATH));
    }

    std::vector<unsigned char> plaintext;
    int value;
    while (infile >> value) {
        if (value < -256 || value > 255) {
            throw std::runtime_error("Error: Query file contains invalid value (must be in range -256 to 255).");
        }
        plaintext.push_back(static_cast<unsigned char>(value));
    }
    infile.close();

    // Check if exactly 512 values were read
    if (plaintext.size() != VECTOR_DIM) {
        throw std::runtime_error("Error: Query file must contain exactly 512 values. Found " + std::to_string(plaintext.size()) + ".");
    }

    return plaintext;
}

// Function to encrypt receiver query
void encrypt_receiver_query() {
    // Create necessary directories
    create_directory("receiver_query");
    create_directory("enc_receiver_query");

    // Read plaintext vector from query file
    std::vector<unsigned char> plaintext = read_plaintext_vector();

    // Generate AES key and IV
    std::vector<unsigned char> key = generate_aes_key();
    std::vector<unsigned char> iv = generate_iv();

    // Store the AES key and IV
    write_aes_key_iv(AES_KEY_FILE, AES_IV_FILE, key, iv);

    // Encrypt the plaintext and measure encryption time
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> ciphertext = aes_encrypt_ctr(plaintext, key, iv);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> encryption_time = end_time - start_time;

    // Write encrypted data to file
    write_encrypted_file(ciphertext, iv);

    // Print results
    /*
    std::cout << "Encrypted vector of 512 values from query file." << std::endl;
    std::cout << "AES Key and IV have been saved in aes_keys/ directory." << std::endl;
    std::cout << "Ciphertext Size: " << ciphertext.size() << " bytes" << std::endl;
    std::cout << "Encryption Time: " << encryption_time.count() << " ms" << std::endl;
    */
    std::cout << ciphertext.size() << " ";
    std::cout << encryption_time.count() << " " << std::endl;

}

int main() {
    try {
        encrypt_receiver_query();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

}
