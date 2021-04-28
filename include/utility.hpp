#pragma once
#include <types.hpp>
#include <string>
#include <random>

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double epsilon = std::numeric_limits<double>::epsilon();
const double neg_epsilon = -std::numeric_limits<double>::epsilon();
const double pi = 3.1415926535897932385;
constexpr double bias = 0.00001;

// helper function
inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}


template<typename Numeric = double, typename Generator = std::mt19937>
Numeric random(Numeric from, Numeric to)
{
    thread_local static Generator gen(std::random_device{}());

    using dist_type = typename std::conditional
        <
        std::is_integral<Numeric>::value
        , std::uniform_int_distribution<Numeric>
        , std::uniform_real_distribution<Numeric>
        >::type;

    thread_local static dist_type dist;

    return dist(gen, typename dist_type::param_type{ from, to });
};


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

inline int random_int(int min, int max) {
    // Returns a random integer in [min, max]
    return static_cast<int>(random_double(min, max + 1));
}

inline bool near_zero(const vec3& v) {
    //constexpr auto zero = 1e-9;
    return (fabs(v[0]) < epsilon) && (fabs(v[1]) < epsilon) && (fabs(v[2]) < epsilon);
}

vec3 generate_random_vec(const double min, const double max);
vec3 random_unit_in_sphere();
vec3 random_unit_vector();
vec3 random_unit_vector(const double min, const double max);
vec3 random_in_hemisphere(const vec3& normal);
vec3 random_unit_in_disk();
color random_color();
color random_color(const double min, const double max);


/* pixel convert */
void RGBPixel(byte pixels[3], const vec3& lightcolor);
/* pixel convert */
void RGBPixel(icolor& color, const vec3& lightcolor);

/* Anti-Aliasing pixel convert */
void AA_RGBPixel(color& pixels, const vec3& lightcolor, const size_t sample_per_pixel, const double gamma);