#pragma once
#include <Intersect.hpp>
#include <cassert>

class Sphere : public IIntersect
{
public:
	Sphere() : center(0.0), radius(0.0) {}
	Sphere(const point3 c, double r, shared_ptr<Material> m) : center(c), radius(r), material(m) {}

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

	double Radius() const { return radius; }
	double Radius2() const { return radius * radius; }
	point3 Center() const { return center; }

	void set_radius(double r) noexcept { radius = r; }
	void set_center(point3 c) noexcept { center = c; }

	static void get_uv(const point3& p, double& u, double& v) {
		auto theta = acos(-p.y);
		auto phi = atan2(-p.z, p.x) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
	}
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
	get_uv(outward_normal, irec.uv.x, irec.uv.y);
	irec.material = material;

	return true;
}

bool Sphere::bounding_box(double time0, double time1, AABB& output_box) const
{
	output_box = AABB(center - vec3(radius, radius, radius),
					  center + vec3(radius, radius, radius));
	return true;
}



class AnimationSphere : public IIntersect
{
public:
	AnimationSphere() : center(0.0), radius(0.0), center_end(0.0), tm0(0.0), tm1(0.0) {}
	AnimationSphere(const point3 c, const point3 c_end, const double time0, const double time1, double r, shared_ptr<Material> m) :
		center(c), center_end(c_end), tm0(time0), tm1(time1), radius(r), material(m) {}

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

	double Radius() const { return radius; }
	double Radius2() const { return radius * radius; }
	point3 Center() const { return center; }
	point3 Center_end() const { return center_end; }

	void set_radius(double r) noexcept { radius = r; }
	void set_center(point3 c) noexcept { center = c; }
	void set_center_end(point3 c) noexcept { center_end = c; }
	point3 move_center(double time) const;
private:
	point3 center, center_end;
	double radius;
	shared_ptr<Material> material;
	/* animation time */
	double tm0, tm1;
};


point3 AnimationSphere::move_center(double time) const
{
	return center + ((time - tm0) / (tm1 - tm0)) * (center_end - center);
}


bool AnimationSphere::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const
{
	vec3 oc = ray.origin() - move_center(ray.time());
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
	vec3 outward_normal = (irec.p - move_center(ray.time())) / radius;
	irec.set_face_normal(ray, outward_normal);
	irec.material = material;

	return true;
}


bool AnimationSphere::bounding_box(double time0, double time1, AABB& output_box) const
{
	AABB box0(move_center(time0) - vec3(radius, radius, radius),
			  move_center(time0) + vec3(radius, radius, radius));

	AABB box1(move_center(time1) - vec3(radius, radius, radius),
			  move_center(time1) + vec3(radius, radius, radius));
	output_box = surrounding_box(box0, box1);
	return true;
}