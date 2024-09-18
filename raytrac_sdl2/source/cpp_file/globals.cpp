#include "globals.h"

std::mutex mtx;
std::atomic<int> completed_pixels(0);
std::atomic<bool> rendering_complete(false);
const double min_distance = 0.1;  // Minimum mesafe
const double max_distance = 1000.0;  // Maksimum mesafe
const double aspect_ratio = 16.0 / 9.0; // Sabit olarak double türünde tanýmlýyoruz
const int image_width = 1280;
const int image_height = static_cast<int>(image_width / aspect_ratio);
const double EPSILON = 1e-6f;
const int MAX_DEPTH = 10;
std::atomic<int> next_row(0);
const double infinity = std::numeric_limits<double>::max();
