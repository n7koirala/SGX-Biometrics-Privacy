#ifndef FILE_PATHS_H
#define FILE_PATHS_H

#include <string>

// Receiver query file paths
const std::string RECEIVER_FILE = "receiver_query/receiver_set.txt";  
const std::string ENCRYPTED_FILE = "enc_receiver_query/receiver_set.enc";  

// AES key and IV storage paths
const std::string AES_KEY_FILE = "aes_keys/aes_key.bin";  
const std::string AES_IV_FILE = "aes_keys/aes_iv.bin";  

// Sender dataset file paths  
const std::string SENDER_FOLDER = "sender_db/";
const std::string SENDER_FILE = "sender_db/sender_set.txt";

// Encrypted receiver query path
const std::string ENC_RECEIVER_FILE = "enc_receiver_query/receiver_set.enc";  

// Encrypted result output path
const std::string ENC_RESULT_FILE = "result/enc_result.enc";  

#endif // FILE_PATHS_H
