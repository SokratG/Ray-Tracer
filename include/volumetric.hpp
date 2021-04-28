#pragma once
#include <Material.hpp>
#include <Intersect.hpp>
#include <cassert>

class ConstantVolume : public IIntersect
{
public:
	ConstantVolume(shared_ptr<IIntersect> bound, const double d, shared_ptr<Texture> tex) : 
		boundary(bound), neg_inv_density(-1 / d), phase_func(make_shared<Isotropic>(tex))  {}
	ConstantVolume(shared_ptr<IIntersect> bound, const double d, const color c) : 
		boundary(bound), neg_inv_density(-1 / d), phase_func(make_shared<Isotropic>(c)) {}

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const override;

	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		return boundary->bounding_box(time0, time1, output_box);
	}
public:
	shared_ptr<IIntersect> boundary;
	shared_ptr<Material> phase_func;
	double neg_inv_density;
};


bool ConstantVolume::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irec) const
{
	IntersectRecord irc1, irc2;

	if (!boundary->intersect(ray, -infinity, infinity, irc1))
		return false;

	if (!boundary->intersect(ray, irc1.t + 0.0001, infinity, irc2))
		return false;


	if (irc1.t < t_min) irc1.t = t_min;
	if (irc2.t > t_max) irc2.t = t_max;

	if (irc1.t >= irc2.t)
		return false;

	if (irc1.t < 0) irc1.t = 0;
		
	const auto ray_length = ray.direction().length();
	const auto dist_inside_boundary = (irc2.t - irc1.t) * ray_length;
	const auto hit_dist = neg_inv_density * glm::log(random_double());

	if (hit_dist > dist_inside_boundary)
		return false;

	irec.t = irc1.t + hit_dist / ray_length;
	irec.p = ray.at(irec.t);
	irec.normal = vec3(1, 0, 0);  
	irec.front_face = true;     
	irec.material = phase_func;

	return true;
}