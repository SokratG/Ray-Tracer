#pragma once
#include <Intersect.hpp>
#include <cassert>

class Sphere : public IIntersect
{
public:
	Sphere() : center(0.0), radius(0.0) {}
	Sphere(const point3 c, double r, shared_ptr<Material> m) : center(c), radius(r), material(m) {}

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const override;

	double Radius() const { return radius; }
	double Radius2() const { return radius * radius; }
	point3 Center() const { return center; }
	void setRadius(double r) noexcept { radius = r; }
	void setCenter(point3 c) noexcept { center = c; }

private:
	point3 center;
	double radius;
	shared_ptr<Material> material;
};

bool Sphere::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const
{
	vec3 oc = ray.origin() - center;
	auto a = glm::length2(ray.direction());
	auto half_b = glm::dot(oc, ray.direction());
	auto c = glm::length2(oc) - Radius2();
	
	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0)
		return false;
	auto sqrtd = sqrt(discriminant);

	// Find the nearest root that lies in the acceptable range.
	auto root = (-half_b - sqrtd) / a;
	if (root < t_min || root > t_max) {
		root = (-half_b + sqrtd) / a;
		if (root < t_min || root > t_max)
			return false;
	}

	irec.t = root;
	irec.p = ray.at(irec.t);
	vec3 outward_normal = (irec.p - center) / radius;
	irec.set_face_normal(ray, outward_normal);
	irec.material = material;

	return true;
}