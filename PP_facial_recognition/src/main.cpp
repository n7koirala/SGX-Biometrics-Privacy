#include "../include/vector_utils.h"
#include "../include/config.h"

int main(int argc, char *argv[]){

    // Open input file
    std::ifstream fileStream;
    if (argc > 1) {
        fileStream.open(argv[1], std::ios::in);
    } else {
        fileStream.open("test/2^10.dat", std::ios::in);
    }

    if (!fileStream.is_open()) {
        std::cerr << "Error: input file not found" << std::endl;
        return 1;
    }

    size_t numVectors;
    fileStream >> numVectors;

    // Read in query vector from file, normalize in place
    std::vector<double> queryVector(VECTOR_DIM);
    for (size_t i = 0; i < VECTOR_DIM; i++) {
        fileStream >> queryVector[i];
    }
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
    std::cout << "Index:     \tThere exist matches at database indices: [ ";
    for (auto i: matchIndices) {
        std::cout << i << ' ';
    }
    std::cout << " ]" << std::endl;

    // compute and output results of membership scenario
    if(VectorUtils::membershipScenario(DELTA, scoreVector, numVectors)) {
        std::cout << "Membership:\tThere exists at least one match between the query template and the database" << std::endl;
    } else {
        std::cout << "Membership:\tThere does not exist a match between the query template and the database" << std::endl;
    }

    return 0;
}