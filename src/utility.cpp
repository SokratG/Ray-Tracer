#include "utility.hpp"

#include <cassert>


void AA_RGBPixel(color& pixels, const vec3& lightcolor, const size_t sample_per_pixel, const double gamma)
{
	assert(gamma > 0);
	auto r = lightcolor[0];
	auto g = lightcolor[1];
	auto b = lightcolor[2];

	auto scale = 1.0 / sample_per_pixel;
	// Divide the color by the number of samples and gamma-correct for gamma correction
	auto gammacorrection = 1.0 / gamma;
	r = glm::pow((r * scale), gammacorrection);
	g = glm::pow((g * scale), gammacorrection);
	b = glm::pow((b * scale), gammacorrection);

	pixels[0] = static_cast<byte>(glm::clamp(r, 0.0, 0.999) * 256);
	pixels[1] = static_cast<byte>(glm::clamp(g, 0.0, 0.999) * 256);
	pixels[2] = static_cast<byte>(glm::clamp(b, 0.0, 0.999) * 256);
}


void RGBPixel(byte pixels[3], const vec3& lightcolor)
{
	pixels[0] = static_cast<byte>(glm::clamp(lightcolor[0], 0.0, 0.999) * 256);
	pixels[1] = static_cast<byte>(glm::clamp(lightcolor[1], 0.0, 0.999) * 256);
	pixels[2] = static_cast<byte>(glm::clamp(lightcolor[2], 0.0, 0.999) * 256);
}

void RGBPixel(icolor& color, const vec3& lightcolor)
{
	color[0] = static_cast<byte>(glm::clamp(lightcolor[0], 0.0, 0.999) * 256);
	color[1] = static_cast<byte>(glm::clamp(lightcolor[1], 0.0, 0.999) * 256);
	color[2] = static_cast<byte>(glm::clamp(lightcolor[2], 0.0, 0.999) * 256);
}


vec3 generate_random_vec(const double min, const double max) {
	return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

vec3 random_unit_in_sphere() {
	while (true) {
		auto p = generate_random_vec(-1.0, 1.0);
		if (glm::length2(p) >= 1)
			continue;
		return p;
	}
}

vec3 random_unit_vector() {
	return glm::normalize(random_unit_in_sphere());
}

vec3 random_in_hemisphere(const vec3& normal)
{
	vec3 unit_vec_sphere = random_unit_in_sphere();
	// In the same hemisphere as the normal
	if (glm::dot(unit_vec_sphere, normal) > 0.0)
		return unit_vec_sphere;
	else
		return -unit_vec_sphere;
}


vec3 random_unit_in_disk()
{
	while (true) {
		auto p = vec3(random_double(-1.0, 1.0), random_double(-1.0, 1.0), 0.0);
		if (glm::length2(p) >= 1.0)
			continue;
		return p;
	}
}

color random_color()
{
	return color(random_double(), random_double(), random_double());
}

color random_color(const double min,const double max)
{
	return color(random_double(min, max), random_double(min, max), random_double(min, max));
}