#pragma once
#include <Intersect.hpp>
#include <cassert>

class Rect
{
public:
	Rect(shared_ptr<Material> _mp, const double _k) : mp(_mp), k(_k) {}
	virtual ~Rect() = 0;
public:
	shared_ptr<Material> mp;
	double k;
};

inline Rect::~Rect() {}


/* ============================================================= */
class xyRect : public Rect, public IIntersect
{
public:
	xyRect() : Rect(nullptr, 0.0), x0(0.0), x1(0.0), y0(0.0), y1(0.0) {}
	xyRect(const double _x0, const double _x1, const double _y0, const double _y1, const double _k,
		shared_ptr<Material> mat) : Rect(mat, _k), x0(_x0), x1(_x1), y0(_y0), y1(_y1) {}
	
public:
	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		// The bounding box must have non-zero width in each dimension, addd to Z dimension a small amount
		output_box = AABB(point3(x0, y0, k - 0.0001), point3(x1, y1, k + 0.0001));
		return true;
	}
public:
	double x0, x1, y0, y1;
};


bool xyRect::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	// t = (k - Az) / bz - from ray equation
	auto t = (k - ray.origin().z) / ray.direction().z;
	if (t < t_min || t > t_max)
		return false;

	auto x = ray.origin().x + t * ray.direction().x;
	auto y = ray.origin().y + t * ray.direction().y;
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;

	irc.uv.x = (x - x0) / (x1 - x0);
	irc.uv.y = (y - y0) / (y1 - y0);
	irc.t = t;
	auto outward_normal = vec3(0, 0, 1);
	irc.set_face_normal(ray, outward_normal);
	irc.material = mp;
	irc.p = ray.at(t);

	return true;
}


/* ============================================================= */
class xzRect : public Rect, public IIntersect
{
public:
	xzRect() : Rect(nullptr, 0.0), x0(0.0), x1(0.0), z0(0.0), z1(0.0) {}
	xzRect(const double _x0, const double _x1, const double _z0, const double _z1, const double _k,
		shared_ptr<Material> mat) : Rect(mat, _k), x0(_x0), x1(_x1), z0(_z0), z1(_z1) {}

public:
	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;

	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		output_box = AABB(point3(x0, k - 0.0001, z0), point3(x1, k + 0.0001, z1));
		return true;
	}
public:
	double x0, x1, z0, z1;
};


bool xzRect::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	auto t = (k - ray.origin().y) / ray.direction().y;
	if (t < t_min || t > t_max)
		return false;

	auto x = ray.origin().x + t * ray.direction().x;
	auto z = ray.origin().z + t * ray.direction().z;
	if (x < x0 || x > x1 || z < z0 || z > z1)
		return false;

	irc.uv.x = (x - x0) / (x1 - x0);
	irc.uv.y = (z - z0) / (z1 - z0);
	irc.t = t;
	auto outward_normal = vec3(0, 1, 0);
	irc.set_face_normal(ray, outward_normal);
	irc.material = mp;
	irc.p = ray.at(t);

	return true;
}



/* ============================================================= */
class yzRect : public Rect, public IIntersect
{
public:
	yzRect() : Rect(nullptr, 0.0), z0(0.0), z1(0.0), y0(0.0), y1(0.0) {}
	yzRect(const double _y0, const double _y1, const double _z0, const double _z1, const double _k,
		shared_ptr<Material> mat) : Rect(mat, _k), y0(_y0), y1(_y1), z0(_z0), z1(_z1) {}

public:
	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		output_box = AABB(point3(k - 0.0001, y0, z0), point3(k + 0.0001, y1, z1));
		return true;
	}
public:
	double y0, y1, z0, z1;
};


bool yzRect::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	auto t = (k - ray.origin().x) / ray.direction().x;
	if (t < t_min || t > t_max)
		return false;

	auto y = ray.origin().y + t * ray.direction().y;
	auto z = ray.origin().z + t * ray.direction().z;
	if (y < y0 || y > y1 || z < z0 || z > z1)
		return false;

	irc.uv.x = (y - y0) / (y1 - y0);
	irc.uv.y = (z - z0) / (z1 - z0);
	irc.t = t;
	auto outward_normal = vec3(1, 0, 0);
	irc.set_face_normal(ray, outward_normal);
	irc.material = mp;
	irc.p = ray.at(t);

	return true;
}
