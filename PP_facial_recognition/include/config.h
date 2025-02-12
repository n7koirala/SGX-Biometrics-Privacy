#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <chrono>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>  // Required for memcpy
#include "aes_crypt.h"
#include "file_paths.h"
#include <cmath>

// Dimension of template vectors taken from dataset
const size_t VECTOR_DIM = 512;

// Match threshold used in cosine similarity comparisons
const double DELTA = 0.85;