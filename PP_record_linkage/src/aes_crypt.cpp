#include "aes_crypt.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>
#include <fstream>
#include <cstring>

// Function to generate a random AES key
std::vector<unsigned char> generate_aes_key() {
    std::vector<unsigned char> key(AES_KEY_SIZE);
    RAND_bytes(key.data(), AES_KEY_SIZE);
    return key;
}

// Function to generate a random IV
std::vector<unsigned char> generate_iv() {
    std::vector<unsigned char> iv(IV_SIZE);
    RAND_bytes(iv.data(), IV_SIZE);
    return iv;
}

// Function to write AES key and IV to files
bool write_aes_key_iv(const std::string& key_file, const std::string& iv_file, 
                      const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv) {
    std::ofstream key_out(key_file, std::ios::binary);
    std::ofstream iv_out(iv_file, std::ios::binary);

    if (!key_out.is_open() || !iv_out.is_open()) {
        std::cerr << "Error writing AES key or IV file." << std::endl;
        return false;
    }

    key_out.write((char*)key.data(), key.size());
    iv_out.write((char*)iv.data(), iv.size());

    key_out.close();
    iv_out.close();
    return true;
}

// Function to read AES key and IV from files
bool read_aes_key_iv(const std::string& key_file, const std::string& iv_file, 
                     std::vector<unsigned char>& key, std::vector<unsigned char>& iv) {
    std::ifstream key_in(key_file, std::ios::binary);
    std::ifstream iv_in(iv_file, std::ios::binary);

    if (!key_in.is_open() || !iv_in.is_open()) {
        std::cerr << "Error reading AES key or IV file." << std::endl;
        return false;
    }

    key.resize(AES_KEY_SIZE);
    iv.resize(IV_SIZE);

    key_in.read((char*)key.data(), AES_KEY_SIZE);
    iv_in.read((char*)iv.data(), IV_SIZE);

    key_in.close();
    iv_in.close();
    return true;
}

// AES-128 Encryption (CTR Mode)
std::vector<unsigned char> aes_encrypt_ctr(const std::vector<unsigned char>& plaintext, 
                                           const std::vector<unsigned char>& key, 
                                           std::vector<unsigned char>& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> ciphertext(plaintext.size());
    int len = 0, ciphertext_len = 0;

    EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, key.data(), iv.data());
    EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size());
    ciphertext_len = len;
    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);
    EVP_CIPHER_CTX_free(ctx);
    return ciphertext;
}

// AES-128 Decryption (CTR Mode)
std::vector<unsigned char> aes_decrypt_ctr(const std::vector<unsigned char>& ciphertext, 
                                           const std::vector<unsigned char>& key, 
                                           std::vector<unsigned char>& iv) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    std::vector<unsigned char> plaintext(ciphertext.size());
    int len = 0, plaintext_len = 0;

    EVP_DecryptInit_ex(ctx, EVP_aes_128_ctr(), NULL, key.data(), iv.data());
    EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext.size());
    plaintext_len = len;
    EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
    plaintext_len += len;

    plaintext.resize(plaintext_len);
    EVP_CIPHER_CTX_free(ctx);
    return plaintext;
}
