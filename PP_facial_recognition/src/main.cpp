#include "vector_utils.h"
#include <ctime>

#include "aes_crypt.h"
#include "file_paths.h"
#include "config.h"


// Function to create a directory if it doesn't exist
void create_directory(const std::string& path) {
    mkdir(path.c_str(), 0777);
}

// Function to write encrypted matchIndices & membership result to file
void write_encrypted_result(const std::vector<unsigned char>& ciphertext, 
                            const std::vector<unsigned char>& iv) {
    std::ofstream outfile(ENC_RESULT_FILE, std::ios::binary);
    if (!outfile) {
        std::cerr << "Error: Could not open file " << ENC_RESULT_FILE << " for writing." << std::endl;
        return;
    }
    // Write IV first
    outfile.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE);
    // Then write the ciphertext
    outfile.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext.size());
    outfile.close();
}


// Function to encrypt matchIndices & membership result
bool encrypt_result(const std::vector<size_t>& matchIndices, bool membership) {
    // Ensure result directory exists
    create_directory("result");

    // Convert matchIndices & membership into a byte array
    std::vector<unsigned char> plaintext;

    // Store the number of indices (as 4-byte integer)
    uint32_t numIndices = matchIndices.size();
    plaintext.insert(plaintext.end(), reinterpret_cast<unsigned char*>(&numIndices),
                     reinterpret_cast<unsigned char*>(&numIndices) + sizeof(uint32_t));

    // Store each index (as 4-byte integers)
    for (size_t index : matchIndices) {
        uint32_t idx = static_cast<uint32_t>(index);
        plaintext.insert(plaintext.end(), reinterpret_cast<unsigned char*>(&idx),
                         reinterpret_cast<unsigned char*>(&idx) + sizeof(uint32_t));
    }

    // Store membership boolean (1 byte)
    plaintext.push_back(static_cast<unsigned char>(membership ? 1 : 0));

    // Load AES key and IV
    std::vector<unsigned char> key, iv;
    if (!read_aes_key_iv(AES_KEY_FILE, AES_IV_FILE, key, iv)) {
        return false;
    }

    // Encrypt the plaintext and measure encryption time
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<unsigned char> ciphertext = aes_encrypt_ctr(plaintext, key, iv);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> encryption_time = end_time - start_time;

    // Write encrypted data to file
    write_encrypted_result(ciphertext, iv);

    
    // Print results
    /*
    std::cout << "Encrypted matchIndices & membership result." << std::endl;
    std::cout << "Stored in: " << ENC_RESULT_FILE << std::endl;
    std::cout << "Ciphertext Size: " << ciphertext.size() << " bytes" << std::endl;
    std::cout << "Encryption Time: " << encryption_time.count() << " ms" << std::endl;
    */

    std::cout << encryption_time.count() << " ";
    std::cout << ciphertext.size() << std::endl;

    return true;
}


// Function to read and decrypt the receiver query
bool decrypt_receiver_query(std::vector<double>& queryVector) {
    // Load AES key and IV
    std::vector<unsigned char> key, iv;
    if (!read_aes_key_iv(AES_KEY_FILE, AES_IV_FILE, key, iv)) {
        std::cerr << "Error: Unable to read AES key or IV!" << std::endl;
        return false;
    }

    // Open the encrypted file
    std::ifstream enc_file(ENCRYPTED_FILE, std::ios::binary);
    if (!enc_file.is_open()) {
        std::cerr << "Error opening encrypted receiver file: " << ENCRYPTED_FILE << std::endl;
        return false;
    }

    // Read IV (first 16 bytes)
    enc_file.read(reinterpret_cast<char*>(iv.data()), IV_SIZE);
    if (static_cast<size_t>(enc_file.gcount()) != IV_SIZE) {
        std::cerr << "Error: IV size mismatch in encrypted file!" << std::endl;
        return false;
    }

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

    // Ensure the decrypted data has exactly 512 values (i.e., 512 bytes)
    if (decrypted_data.size() != VECTOR_DIM) {
        std::cerr << "Error: Decrypted data size mismatch! Expected " << VECTOR_DIM 
                  << " values but got " << decrypted_data.size() << std::endl;
        return false;
    }

    // Copy decrypted values into queryVector
    queryVector.resize(VECTOR_DIM);
    for (size_t i = 0; i < VECTOR_DIM; i++) {
        queryVector[i] = static_cast<double>(decrypted_data[i]); // Convert bytes to double
    }

    // Print decryption results
    /*
    std::cout << "Successfully decrypted " << VECTOR_DIM << " values." << std::endl;
    std::cout << "Decryption Time: " << decryption_time.count() << " ms" << std::endl;
    */
    std::cout << decryption_time.count() << " ";

    return true;
}


int main(int argc, char *argv[]){

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::duration<double> elapsed;

    // Open input file
    if (argc <= 1) {
        // Throwing an exception:
        throw std::runtime_error("Error: No input dataset provided. Usage: make <input_file>");
        
        // Alternatively, you can print an error message and exit:
        // std::cerr << "Error: No input file provided. Usage: " << argv[0] << " <input_file>" << std::endl;
        // return 1;
    }

    // Open the file provided in the first argument

    std::ifstream fileStream(argv[1], std::ios::in);

    if (!fileStream.is_open()) {
        std::cerr << "Error: input file not found" << std::endl;
        return 1;
    }

    if (!fileStream.is_open()) {
        std::cerr << "Error: input file not found" << std::endl;
        return 1;
    }

    size_t numVectors;
    fileStream >> numVectors;

    // Read in query vector from file, normalize in place
    std::vector<double> queryVector(VECTOR_DIM);

    // Decrypt the receiver query and populate queryVector
    if (!decrypt_receiver_query(queryVector)) {
        return 1;  // Exit with failure
    }

    // query time 
    start = std::chrono::steady_clock::now();
    VectorUtils::normalizeInPlace(queryVector, VECTOR_DIM);

    // Read in database vectors from file, normalize in place
    std::vector<std::vector<double>> databaseVectors(numVectors, std::vector<double>(VECTOR_DIM));
    for (size_t i = 0; i < numVectors; i++) {
        for (size_t j = 0; j < VECTOR_DIM; j++) {
            fileStream >> databaseVectors[i][j];
        }
        VectorUtils::normalizeInPlace(databaseVectors[i], VECTOR_DIM);
    }
    fileStream.close();

    // Query operations begin here
    std::vector<double> scoreVector(numVectors);

    // compute inner product (cosine sim) between query and all database vectors
    // TODO: parallelize with OpenMP
    
    for(size_t i = 0; i < numVectors; i++) {
        scoreVector[i] = VectorUtils::innerProduct(queryVector, databaseVectors[i], VECTOR_DIM);
    }

    // compute and output results of index scenario
    std::vector<size_t> matchIndices = VectorUtils::indexScenario(DELTA, scoreVector, numVectors);
    bool membership = VectorUtils::membershipScenario(DELTA, scoreVector, numVectors);

    stop = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << elapsed.count() << " ";

    // std::cout << "Index:     \tThere exist matches at database indices: [ ";
    /*
    for (auto i: matchIndices) {
        std::cout << i << " ";
    }

    // compute and output results of membership scenario
    if(membership) {
        std::cout << 1 << std::endl;
        // std::cout << "Membership:\tThere exists at least one match between the query template and the database" << std::endl;
    } else {
        std::cout << 0 << std::endl;
        // std::cout << "Membership:\tThere does not exist a match between the query template and the database" << std::endl;
    }
    */

    // Encrypt and save the result
    if (!encrypt_result(matchIndices, membership)) {
        std::cerr << "Error encrypting result.\n";
        return 1;
    }

    return 0;
}