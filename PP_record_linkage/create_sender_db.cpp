#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

const int NUM_RECORDS = 16384;
const std::string SENDER_FOLDER = "sender_db/";

void populate_senders_sets(int num_senders) {
    // Create sender_db directory if it doesn't exist
    mkdir(SENDER_FOLDER.c_str(), 0777);

    std::srand(std::time(0));
    for (int i = 0; i < num_senders; ++i) {
        std::string sender_file_name = SENDER_FOLDER + "sender_set_" + std::to_string(i + 1) + ".txt";
        std::ofstream sender_file(sender_file_name);
        if (!sender_file.is_open()) {
            std::cerr << "Error creating sender file: " << sender_file_name << std::endl;
            continue;
        }
        for (int j = 0; j < NUM_RECORDS; ++j) {
            uint32_t value = std::rand();
            sender_file << value << "\n";
        }
        sender_file.close();
    }
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

    populate_senders_sets(num_senders);

    return 0;
}
