#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// Dimension of template vectors taken from dataset
const size_t VECTOR_DIM = 512;

// Match threshold used in cosine similarity comparisons
const double DELTA = 0.85;