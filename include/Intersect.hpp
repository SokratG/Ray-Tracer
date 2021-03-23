#pragma once
#include <vector>
#include <utility.hpp>
#include <Ray.hpp>

class Material;

/*
	class IntersectRecord - contain some detail about intersect with surface
*/
struct IntersectRecord
{
	point3 p; // intersect point
	vec3 normal; // normal vector
	shared_ptr<Material> material;
	double t; // parameter
	bool front_face = false;
	inline void set_face_normal(const Ray& ray, const vec3& outward_normal) {
		front_face = glm::dot(ray.direction(), outward_normal) < 0.0; // determining position intersection ray and normal surface
		normal = front_face ? outward_normal : -outward_normal; // set normal to opposite of the ray
	}
};

/*
	interface IIntersect - provide interface for computin intersectin shapes
*/
class IIntersect
{
public:
	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const = 0;
};




class IntersectList : public IIntersect
{
public:
	IntersectList() {}
	IntersectList(shared_ptr<IIntersect> object) { add(object); }

	IntersectList(const IntersectList&) = delete;
	IntersectList& operator=(const IntersectList&) = delete;
	
	void clear() { objects.clear(); }
	void add(shared_ptr<IIntersect> object) { objects.push_back(object); }

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;
private:
	std::vector<shared_ptr<IIntersect>> objects; // array with intersection shapes
};

bool IntersectList::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const
{
	IntersectRecord temp_irc;
	bool is_intersect = false;
	auto closest_dist = t_max; 

	for (const auto& object : objects) {
		if (object->intersect(ray, t_min, closest_dist, temp_irc)) {
			is_intersect = true;
			closest_dist = temp_irc.t;
			ir = temp_irc;
		}
	}

	return is_intersect;
}


using intersect_record = IntersectRecord;
using IntersectionList = IntersectList;
//using World = IntersectList;