#include "../include/vector_utils.h"

void VectorUtils::normalizeInPlace(std::vector<double> &vec, size_t vectorDim) {
    // compute magnitude of vec
    double magnitude = 0.0;
    for(size_t i = 0; i < vectorDim; i++) {
        magnitude += (vec[i] * vec[i]);
    }
    magnitude = sqrt(magnitude);

    // divide each element in vec by magnitude
    if (magnitude != 0) {
        for (size_t i = 0; i < vectorDim; i++) {
            vec[i] = vec[i] / magnitude;
        }
    }
}

double VectorUtils::innerProduct(std::vector<double> va, std::vector<double> vb, size_t vectorDim) {
    double prod = 0.0;
    for(size_t i = 0; i < vectorDim; i++) {
        prod += (va[i] * vb[i]);
    }
    return prod;
}

bool VectorUtils::membershipScenario(double threshold, std::vector<double> scoreVec, size_t numVectors) {
    bool flag = false;

    // TODO: parallelize
    for(size_t i = 0; i < numVectors; i++) {
        if(scoreVec[i] >= threshold) {
            flag = true;
        }
    }
    return flag;
}
    
std::vector<size_t> VectorUtils::indexScenario(double threshold, std::vector<double> scoreVec, size_t numVectors) {
    std::vector<size_t> matchIndices(0);

    // TODO: parallelize
    for(size_t i = 0; i < numVectors; i++) {
        if(scoreVec[i] >= threshold) {
            matchIndices.push_back(i);
        }
    }
    return matchIndices;
}