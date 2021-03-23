#pragma once
#include <types.hpp>
#include <string>
#include <random>

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;
const double zero = std::numeric_limits<double>::epsilon();

// helper function
inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

#ifdef off_cpp
inline double random_double() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}
#endif


inline double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min) * random_double();
}

inline bool near_zero(const vec3& v) {
    //constexpr auto zero = 1e-9;
    return (fabs(v[0]) < zero) && (fabs(v[1]) < zero) && (fabs(v[2]) < zero);
}

vec3 generate_random_vec(const double min, const double max);
vec3 random_unit_in_sphere();
vec3 random_unit_vector();
vec3 random_in_hemisphere(const vec3& normal);
vec3 random_unit_in_disk();
color random_color();
color random_color(const double min, const double max);

void save_img(const std::string& filename, byte* data, size_t width, size_t height, size_t num_ch);

/* pixel convert */
void RGBPixel(byte pixels[3], const vec3& lightcolor);
/* pixel convert */
void RGBPixel(icolor& color, const vec3& lightcolor);

/* Anti-Aliasing pixel convert */
void AA_RGBPixel(byte pixels[3], const vec3& lightcolor, const size_t sample_per_pixel, const double gamma);