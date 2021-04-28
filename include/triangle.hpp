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
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

	void set_points(const point3& p1, const point3& p2, const point3& p3);
	void set_material(const shared_ptr<Material> m) { material = m; }

	void get_uv(const point3& p, double& u, double& v) const;
public:
	vec3 normal() const;
	bool get_barycentric_coord(const Ray& ray, barycentric& uvw) const;
	bool get_barycentric_coord(const point3& pt, barycentric& uvw) const;
private:
	vec3 get_normal() const;
	bool check_point_in(const point3& P, barycentric& uvw) const;
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
	
	// close to zero - parallel
	if (det < epsilon && det > neg_epsilon) {
		return false;
	}

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
	
	// auto w = 1 - u - v;

	irec.t = t;
	irec.p = ray.at(irec.t);
	vec3 outward_normal = get_normal();
	irec.uv.x = u; // u
	irec.uv.y = v; // v
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

bool Triangle::check_point_in(const point3& P, barycentric& uvw) const
{
	vec3 N = get_normal();
	// inside-outside test (CCW point P on left side)
	point3 _C(0.0);

	// edge - 1
	auto AB = B - A;
	auto AP = P - A;
	_C = glm::cross(AB, AP);

	if (glm::dot(N, _C) < 0.0) // P on right side
		return false;

	// edge - 2
	auto BC = C - B;
	auto BP = P - B;
	_C = glm::cross(BC, BP);
	auto u = glm::dot(N, _C);
	if (u < 0.0) // P on right side
		return false;

	// edge - 3
	auto CA = A - C;
	auto CP = P - C;
	_C = glm::cross(CA, CP);
	auto v = glm::dot(N, _C);
	if (v < 0.0) // P on right side
		return false;

	const auto denom = glm::dot(N, N);

	u /= denom;
	v /= denom;

	uvw.x = u;
	uvw.y = v;
	uvw.z = 1.0 - (u + v);

	return true;
}


bool Triangle::get_barycentric_coord(const Ray& ray, barycentric& uvw) const
{
	const vec3 edge1 = B - A;
	const vec3 edge2 = C - A;
	const vec3 q = glm::cross(ray.direction(), edge2);

	const double a = glm::dot(edge1, q);

	/* check parallel or behind the triangle */
	if (fabs(a) <= epsilon)
		return false;

	const vec3 s = ray.origin() - A;
	const vec3 r = glm::cross(s, edge1);

	uvw.x = glm::dot(s, q) / a;
	uvw.y = glm::dot(ray.direction(), r) / a;
	uvw.z = 1.0 - (uvw.x + uvw.y);

	return true;
}


bool Triangle::get_barycentric_coord(const point3& pt, barycentric& uvw) const
{
	return check_point_in(pt, uvw);
}



bool Triangle::bounding_box(double time0, double time1, AABB& output_box) const
{
	auto min_x = std::min({ A.x, B.x, C.x });
	auto min_y = std::min({ A.y, B.y, C.y });
	auto min_z = std::min({ A.z, B.z, C.z });

	auto max_x = std::max({ A.x, B.x, C.x });
	auto max_y = std::max({ A.y, B.y, C.y });
	auto max_z = std::max({ A.z, B.z, C.z });

	point3 a(min_x, min_y, min_z);
	point3 b(max_x, max_y, max_z);

	output_box = AABB(a, b);

	return true;
}


void Triangle::get_uv(const point3& P, double& u, double& v) const
{
	/* 
		fast barycentric compute coordinates 
		https://gamedev.stackexchange.com/questions/23743/whats-the-most-efficient-way-to-find-barycentric-coordinates
	*/
	vec3 v0 = B - A; vec3 v1 = C - A; vec3 v2 = P - A;
	auto d00 = glm::dot(v0, v0);
	auto d01 = glm::dot(v0, v1);
	auto d11 = glm::dot(v1, v1);
	auto d20 = glm::dot(v2, v0);
	auto d21 = glm::dot(v2, v1);
	auto denom = d00 * d11 - d01 * d01;

	auto alpha = (d11 * d20 - d01 * d21) / denom;
	auto beta = (d00 * d21 - d01 * d20) / denom;
	auto gamma = 1 - alpha - beta;
	u = alpha;
	v = beta;
}