#pragma once

#include "../include/config.h"

namespace VectorUtils {
    void normalizeInPlace(std::vector<double> &vec, size_t vectorDim);
    double innerProduct(std::vector<double> va, std::vector<double> vb, size_t vectorDim);
    bool membershipScenario(double threshold, std::vector<double> scoreVec, size_t numVectors);
    std::vector<size_t> indexScenario(double threshold, std::vector<double> scoreVec, size_t numVectors);
} // namespace VectorUtils