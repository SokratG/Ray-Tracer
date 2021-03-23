#pragma once
#include <types.hpp>


class Ray
{
private:
	point3 orig;
	vec3 dir;
public:
	Ray() : orig(0.0), dir(0.0) {}
	Ray(const point3& orig_, const vec3& dir_) : orig(orig_), dir(dir_) {}
	point3 origin() const { return orig; }
	vec3 direction() const { return dir; }
	point3 at(double t) const { return orig + t * dir; }
	void setorigin(const point3& orig_) noexcept { orig = orig_; }
	void setdir(const vec3& dir_) noexcept { dir = dir_; }
};