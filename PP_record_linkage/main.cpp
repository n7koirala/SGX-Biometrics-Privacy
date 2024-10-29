#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

const int NUM_RECORDS = 16384;
const std::string RECEIVER_FILE = "receiver_query/receiver_set.txt";
const std::string SENDER_FOLDER = "sender_db/";

void populate_receiver_set() {
    // Create receiver_query directory if it doesn't exist
    mkdir("receiver_query", 0777);

    std::ofstream receiver_file(RECEIVER_FILE);
    if (!receiver_file.is_open()) {
        std::cerr << "Error creating receiver file: " << RECEIVER_FILE << std::endl;
        return;
    }
    uint32_t value = std::rand();
    receiver_file << value << "\n";
    receiver_file.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <num_senders>" << std::endl;
        return 1;
    }

    int num_senders = std::atoi(argv[1]);
    if (num_senders <= 0) {
        std::cerr << "Number of senders must be greater than 0." << std::endl;
        return 1;
    }

    //populate_receiver_set();

    std::unordered_set<uint32_t> receiver_set;
    uint32_t receiver_value;

    // Load the receiver set (containing only one record)
    std::ifstream receiver_file(RECEIVER_FILE);
    if (!receiver_file.is_open()) {
        std::cerr << "Error opening receiver file: " << RECEIVER_FILE << std::endl;
        return 1;
    }
    receiver_file >> receiver_value;
    receiver_set.insert(receiver_value);
    receiver_file.close();

    bool found = false;

    // Iterate over each sender set
    for (int i = 0; i < num_senders; ++i) {
        std::string sender_file_name = SENDER_FOLDER + "sender_set_" + std::to_string(i + 1) + ".txt";
        std::ifstream sender_file(sender_file_name);
        if (!sender_file.is_open()) {
            std::cerr << "Error opening sender file: " << sender_file_name << std::endl;
            return 1;
        }

        std::unordered_set<uint32_t> sender_set;
        uint32_t value;
        int count = 0;

        // Load the sender set
        while (count < NUM_RECORDS && sender_file >> value) {
            sender_set.insert(value);
            count++;
        }
        sender_file.close();

        // Perform set intersection with receiver set
        if (sender_set.find(receiver_value) != sender_set.end()) {
            found = true;
            break;
        }
    }

    if (found) {
        std::cout << "Receiver value " << receiver_value << " found in one of the sender sets." << std::endl;
    } else {
        std::cout << "Receiver value " << receiver_value << " NOT found in any of the sender sets." << std::endl;
    }

    return 0;
}
