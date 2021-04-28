#pragma once
#include <utility.hpp>
#include <Ray.hpp>
#include <algorithm>

// Axis-aligned bounding box class
class AABB
{
public:
	AABB() : aa(0.0), bb(0.0) {}
	AABB(const point3 a, const point3 b) : aa(a), bb(b) {}
	point3 min() const { return aa; }
	point3 max() const { return bb; }
	point3 set_min(const point3 a) { aa = a; }
	point3 set_max(const point3 b) { bb = b; }
	bool intersect(const Ray& ray, double t_min, double t_max) const;
protected:
	bool intersect_slow(const Ray& ray, double t_min, double t_max) const;
	bool intersect_fast(const Ray& ray, double t_min, double t_max) const;
private:
	point3 aa;
	point3 bb;
};

bool AABB::intersect(const Ray& ray, double t_min, double t_max) const
{
	return intersect_fast(ray, t_min, t_max);
}

bool AABB::intersect_slow(const Ray& ray, double t_min, double t_max) const
{
	/*
	*	Find Ray-slab overlap interval [t0, t1] for xyz-coordinates:
		t0 = min((x0-Ax)/bx, (x1-Ax)/bx)
		t1 = max((x1-Ax)/bx, (x0-Ax)/bx)
	*/
	for (auto i = 0; i < 3; ++i) { // xyz
		auto t0 = fmin((aa[i] - ray.origin()[i]) / ray.direction()[i], (bb[i] - ray.origin()[i]) / ray.direction()[i]);
		auto t1 = fmax((aa[i] - ray.origin()[i]) / ray.direction()[i], (bb[i] - ray.origin()[i]) / ray.direction()[i]);
		t_min = fmax(t0, t_min);
		t_max = fmin(t1, t_max);
		if (t_max <= t_min)
			return false;
	}
	return true;
}

bool AABB::intersect_fast(const Ray& ray, double t_min, double t_max) const
{
	for (auto i = 0; i < 3; ++i) { // xyz
		auto invDir = 1.0 / ray.direction()[i];
		auto t0 = (min()[i] - ray.origin()[i]) * invDir;
		auto t1 = (max()[i] - ray.origin()[i]) * invDir;
		if (invDir < 0.0)
			std::swap(t0, t1);
		t_min = t0 > t_min ? t0 : t_min;
		t_max = t1 < t_max ? t1 : t_max;
		if (t_max <= t_min)
			return false;
	}
	return true;
}


AABB surrounding_box(const AABB& box0, const AABB& box1) {
	point3 small(fmin(box0.min().x, box1.min().x),
				 fmin(box0.min().y, box1.min().y),
				 fmin(box0.min().z, box1.min().z));

	point3 big(fmax(box0.max().x, box1.max().x),
			   fmax(box0.max().y, box1.max().y),
			   fmax(box0.max().z, box1.max().z));

	return AABB(small, big);
}


