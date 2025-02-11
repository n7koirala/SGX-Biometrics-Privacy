#ifndef AES_CRYPT_H
#define AES_CRYPT_H

#include <vector>
#include <string>

const int AES_KEY_SIZE = 16;  // AES-128 key size (16 bytes)
const int IV_SIZE = 16;       // AES block size (16 bytes)

// Function declarations
std::vector<unsigned char> generate_aes_key();
std::vector<unsigned char> generate_iv();

bool write_aes_key_iv(const std::string& key_file, const std::string& iv_file, 
                      const std::vector<unsigned char>& key, const std::vector<unsigned char>& iv);
bool read_aes_key_iv(const std::string& key_file, const std::string& iv_file, 
                     std::vector<unsigned char>& key, std::vector<unsigned char>& iv);

std::vector<unsigned char> aes_encrypt_ctr(const std::vector<unsigned char>& plaintext, 
                                           const std::vector<unsigned char>& key, 
                                           std::vector<unsigned char>& iv);

std::vector<unsigned char> aes_decrypt_ctr(const std::vector<unsigned char>& ciphertext, 
                                           const std::vector<unsigned char>& key, 
                                           std::vector<unsigned char>& iv);

#endif // AES_CRYPT_H

/*
We are using AES-CTR mode for the AES for our application. Some notes:

No Padding Overhead: Encrypts only 4 bytes (instead of full 16-byte AES block).
No Redundant Ciphertext: Unlike AES-ECB, repeated plaintext values won’t have the same ciphertext.
Fast Performance: CTR mode is highly parallelizable, making it faster than CBC or GCM.
Minimal Ciphertext Size: 4 bytes instead of 16 bytes in AES-ECB or AES-CBC.
*/