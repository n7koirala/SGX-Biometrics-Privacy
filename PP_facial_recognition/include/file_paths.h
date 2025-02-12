#ifndef FILE_PATHS_H
#define FILE_PATHS_H

#include <string>

// Receiver query file paths
const std::string QUERY_FILE_PATH = "receiver_query/query_vector.txt";  
const std::string ENCRYPTED_FILE = "enc_receiver_query/receiver_set.enc";  

const std::string ENC_RECEIVER_FILE = "enc_receiver_query/receiver_set.enc"; 

// AES key and IV storage paths
const std::string AES_KEY_FILE = "aes_keys/aes_key.bin";  
const std::string AES_IV_FILE = "aes_keys/aes_iv.bin";  

// Sender dataset file paths  


// Encrypted result output path
const std::string ENC_RESULT_FILE = "result/enc_result.enc";  

#endif // FILE_PATHS_H