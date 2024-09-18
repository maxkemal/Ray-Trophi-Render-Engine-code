#ifndef GLOBALS_H
#define GLOBALS_H

#include <mutex>
#include <atomic>
#include <limits>
#include <cmath>
#include <vector>
#include "Vec3SIMD.h"

extern std::mutex mtx;
extern std::atomic<int> completed_pixels;
extern std::atomic<bool> rendering_complete;
extern const double min_distance;
extern const double max_distance;
extern const double aspect_ratio; // Sabit olarak double türünde tanýmlýyoruz
extern const int image_width;
extern const int image_height;
extern const double EPSILON;
extern const int MAX_DEPTH;
extern std::atomic<int> next_row;
extern const double infinity;

#endif // GLOBALS_H
