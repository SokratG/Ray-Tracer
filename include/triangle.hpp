#pragma once
#include <Intersect.hpp>
#include <cassert>

/*
 Doug Baldwin and Michael Weber, 
 Fast Ray-Triangle Intersections by Coordinate Transformation, Journal of Computer Graphics Techniques (JCGT), vol. 5, no. 3, 39-49, 2016
*/


class Triangle : public IIntersect
{
public:
	Triangle() : A(0.0), B(0.0), C(0.0) {}
	Triangle(const point3& p1, const point3& p2, const point3& p3, const shared_ptr<Material> m) : A(p1), B(p2), C(p3), material(m) {}
	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const override;
	void set_points(const point3& p1, const point3& p2, const point3& p3);
	void set_material(const shared_ptr<Material> m) { material = m; }
public:
	vec3 normal() const;
private:
	vec3 get_normal() const;
	bool check_point_in(const point3& P) const;
private:
	point3 A, B, C;
	shared_ptr<Material> material;
};


bool Triangle::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const
{
	/* 
		Möller Tomas and Ben Trumbore. "Fast, minimum storage ray-triangle intersection." Journal of graphics tools 2.1 (1997): 21-28.
	*/
	vec3 AB = B - A;
	vec3 AC = C - A;
	auto pvec = glm::cross(ray.direction(), AC);
	const double det = glm::dot(AB, pvec);
	
	if (fabs(det) < zero) // close to zero - parallel
		return false;

	const double inv_det = 1.0 / det;

	auto tvec = ray.origin() - A;
	
	// calculate U parameterand test bounds	
	auto u = glm::dot(tvec, pvec) * inv_det;
	if (u < 0.0 || u > 1.0)
		return false;

	auto qvec = glm::cross(tvec, AB);
	auto v = glm::dot(ray.direction(), qvec) * inv_det;
	if (v < 0.0 || u + v > 1.0)
		return false;

	double t = glm::dot(AC, qvec) * inv_det;
	if (t < t_min || t > t_max) 
		return false;
	
	irec.t = t;
	irec.p = ray.at(irec.t);
	vec3 outward_normal = get_normal();
	irec.set_face_normal(ray, outward_normal);
	irec.material = material;

	return true;
}

vec3 Triangle::normal() const
{
	vec3 normal = glm::cross((B - A), (C - A));
	normal = glm::normalize(normal);
	return normal;
}

vec3 Triangle::get_normal() const
{
	return glm::cross((B - A), (C - A));
}

void Triangle::set_points(const point3& p1, const point3& p2, const point3& p3)
{
	A = p1;
	B = p2;
	C = p3;
}

bool Triangle::check_point_in(const point3& P) const
{
	/*
		feature: add computing barycentric coordinate
	*/
	vec3 N = get_normal();
	// inside-outside test (CCW point P on left side)
	point3 C(0.0);
	auto AB = B - A;
	auto AP = P - A;
	C = glm::cross(AB, AP);

	if (glm::dot(N, C) < 0.0) // P on right side
		return false;

	auto BC = C - B;
	auto BP = P - B;
	C = glm::cross(BC, BP);

	if (glm::dot(N, C) < 0.0) // P on right side
		return false;

	auto CA = A - C;
	auto CP = P - C;
	C = glm::cross(CA, CP);
	if (glm::dot(N, C) < 0.0) // P on right side
		return false;

	return true;
}