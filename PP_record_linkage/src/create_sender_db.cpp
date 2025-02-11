#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include "file_paths.h"


void generate_sender_set(int num_records) {
    // Create sender_db directory if it doesn't exist
    mkdir(SENDER_FOLDER.c_str(), 0777);

    // Open sender file for writing
    std::ofstream sender_file(SENDER_FILE);
    if (!sender_file.is_open()) {
        std::cerr << "Error creating sender file: " << SENDER_FILE << std::endl;
        return;
    }

    std::srand(std::time(0));

    // Generate and write NUM_RECORDS random 32-bit integers
    for (int i = 0; i < num_records; ++i) {
        uint32_t value = std::rand();
        sender_file << value << "\n";
    }

    sender_file.close();
    std::cout << "Successfully generated " << num_records << " records in " << SENDER_FILE << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <NUM_RECORDS>" << std::endl;
        return 1;
    }

    int num_records = std::atoi(argv[1]);
    if (num_records <= 0) {
        std::cerr << "Number of records must be greater than 0." << std::endl;
        return 1;
    }

    generate_sender_set(num_records);

    return 0;
}
