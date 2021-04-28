#pragma once
#include <vector>
#include <AABB.hpp>

class Material;

/*
	class IntersectRecord - contain some detail about intersect with surface
*/
struct IntersectRecord
{
	point3 p; // intersect point
	vec3 normal; // normal vector
	shared_ptr<Material> material;
	vec2 uv;
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
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const = 0;
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
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;
public:
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

bool IntersectList::bounding_box(double time0, double time1, AABB& output_box) const
{
	if (objects.empty())
		return false;

	bool first_box = true;
	AABB res_box;
	for (const auto& object : objects) {
		if (!object->bounding_box(time0, time1, res_box))
			return false;
		output_box = first_box ? res_box : surrounding_box(output_box, res_box);
		first_box = false;
	}

	return true;
}


/*
	Bounding Volume Hierarchy
*/

class BVH_Node : public IIntersect
{
public:
	BVH_Node() {}

	BVH_Node(const IntersectList& ilist, double time0, double time1) :
		BVH_Node(ilist.objects, 0, ilist.objects.size(), time0, time1) {}

	BVH_Node(const std::vector<shared_ptr<IIntersect>>& src_objects,
		size_t start, size_t end, double time0, double time1);

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override;

public:
	shared_ptr<IIntersect> left;
	shared_ptr<IIntersect> right;
	AABB box;
};

bool BVH_Node::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	if (!box.intersect(ray, t_min, t_max))
		return false;
	bool intersect_left = left->intersect(ray, t_min, t_max, irc);
	bool intersect_right = right->intersect(ray, t_min, intersect_left ? irc.t : t_max, irc);

	return intersect_left || intersect_right;
}

bool BVH_Node::bounding_box(double time0, double time1, AABB& output_box) const
{
	output_box = box;
	return true;
}

inline bool box_compare(const shared_ptr<IIntersect> a, const shared_ptr<IIntersect> b, int axis)
{
	AABB box_a;
	AABB box_b;
	if (!a->bounding_box(0, 0, box_a) || !b->bounding_box(0, 0, box_b))
		assert(false && "No bounding box in BVH_Node constructor.\n");

	return box_a.min()[axis] < box_b.min()[axis];
}


bool box_x_compare(const shared_ptr<IIntersect> a, const shared_ptr<IIntersect> b) {
	return box_compare(a, b, 0);
}

bool box_y_compare(const shared_ptr<IIntersect> a, const shared_ptr<IIntersect> b) {
	return box_compare(a, b, 1);
}

bool box_z_compare(const shared_ptr<IIntersect> a, const shared_ptr<IIntersect> b) {
	return box_compare(a, b, 2);
}



BVH_Node::BVH_Node(const std::vector<shared_ptr<IIntersect>>& src_objects,
					size_t start, size_t end, double time0, double time1)
{
	// Create a modifiable array of the source scene objects:
	auto objects = src_objects;
	int axis = random_int(0, 2); // choose random axis
	auto comparator = (axis == 0) ? box_x_compare
					: (axis == 1) ? box_y_compare
								  : box_z_compare;
	size_t object_span = end - start;

	
	if (object_span == 1) {
		// if only one object on the scene - copy them to subtree
		left = right = objects[start];
	}
	else if (object_span == 2) {
		if (comparator(objects[start], objects[start + 1])) {
			left = objects[start];
			right = objects[start + 1];
		}
		else {
			left = objects[start + 1];
			right = objects[start];
		}
	}
	else {
		std::sort(objects.begin() + start, objects.begin() + end, comparator);
		auto mid = start + (object_span >> 1);
		left = make_shared<BVH_Node>(objects, start, mid, time0, time1);
		right = make_shared<BVH_Node>(objects, mid, end, time0, time1);
	}

	AABB box_left, box_right;
	if (!left->bounding_box(time0, time1, box_left) || !right->bounding_box(time0, time1, box_right))
		assert(false && "No bounding box in BVH_Node constructor.\n");

	box = surrounding_box(box_left, box_right);
}





/* ============================================== */
//               Transformation
class Translate : public IIntersect
{
public:
	Translate(shared_ptr<IIntersect> i_p, const vec3& displacement) : i_ptr(i_p), offset(displacement) {}

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		if (!i_ptr->bounding_box(time0, time1, output_box))
			return false;

		output_box = AABB(output_box.min() + offset, output_box.max() + offset);
		return true;
	}

public:
	shared_ptr<IIntersect> i_ptr;
	vec3 offset;
};

bool Translate::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	Ray moved_ray(ray.origin() - offset, ray.direction(), ray.time());
	if (!i_ptr->intersect(moved_ray, t_min, t_max, irc))
		return false;

	irc.p += offset;
	irc.set_face_normal(moved_ray, irc.normal);
	return true;
}



class Rotate : public IIntersect
{
public:
	Rotate(shared_ptr<IIntersect> i_p, const vec3& axis_rot, const double angle);

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		output_box = bbox;
		return hasbox;
	}
public:
	shared_ptr<IIntersect> i_ptr;
	vec3 axis;
	double sin_theta;
	double cos_theta;
	bool hasbox;
	AABB bbox;
};


Rotate::Rotate(shared_ptr<IIntersect> i_p, const vec3& axis_rot, const double angle) : i_ptr(i_p), axis(glm::normalize(axis_rot))
{
	auto rad = glm::radians(angle);
	sin_theta = glm::sin(rad);
	cos_theta = glm::cos(rad);
	hasbox = i_ptr->bounding_box(0, 1, bbox);

	point3 min(infinity, infinity, infinity);
	point3 max(-infinity, -infinity, -infinity);

	for (auto i = 0; i < 2; i++) {
		for (auto j = 0; j < 2; j++) {
			for (auto k = 0; k < 2; k++) {
				auto x = i * bbox.max().x + (1 - i) * bbox.min().x;
				auto y = j * bbox.max().y + (1 - j) * bbox.min().y;
				auto z = k * bbox.max().z + (1 - k) * bbox.min().z;
				
				vec3 tester;

				if (axis.x >= 1) {
					auto newy = cos_theta * y - sin_theta * z;
					auto newz = sin_theta * y + cos_theta * z;
					tester = vec3(x, newy, newz);
				}
				else if (axis.y >= 1) {
					auto newx = cos_theta * x + sin_theta * z;
					auto newz = -sin_theta * x + cos_theta * z;
					tester = vec3(newx, y, newz);
				}
				else {
					auto newx = cos_theta * x - sin_theta * y;
					auto newy = sin_theta * x + cos_theta * y;
					tester = vec3(newx, newy, z);
				}

				for (int c = 0; c < 3; c++) {
					min[c] = fmin(min[c], tester[c]);
					max[c] = fmax(max[c], tester[c]);
				}
			}
		}
	}

	bbox = AABB(min, max);
}


bool Rotate::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	auto orig = ray.origin();
	auto dir = ray.direction();

	/* https://github.com/RayTracing/raytracing.github.io/issues/544 */
	if (axis.x >= 1) {
		orig[1] = cos_theta * ray.origin()[1] + sin_theta * ray.origin()[2];
		orig[2] = -sin_theta * ray.origin()[1] + cos_theta * ray.origin()[2];
		dir[1] = cos_theta * ray.direction()[1] + sin_theta * ray.direction()[2];
		dir[2] = -sin_theta * ray.direction()[1] + cos_theta * ray.direction()[2];
	}
	else if (axis.y >= 1) {
		orig[0] = cos_theta * ray.origin()[0] - sin_theta * ray.origin()[2];
		orig[2] = sin_theta * ray.origin()[0] + cos_theta * ray.origin()[2];
		dir[0] = cos_theta * ray.direction()[0] - sin_theta * ray.direction()[2];
		dir[2] = sin_theta * ray.direction()[0] + cos_theta * ray.direction()[2];
	}
	else {
		orig[0] = cos_theta * ray.origin()[0] + sin_theta * ray.origin()[1];
		orig[1] = -sin_theta * ray.origin()[0] + cos_theta * ray.origin()[1];
		dir[0] = cos_theta * ray.direction()[0] + sin_theta * ray.direction()[1];
		dir[1] = -sin_theta * ray.direction()[0] + cos_theta * ray.direction()[1];
	}

	Ray r_rot(orig, dir, ray.time());

	if (!i_ptr->intersect(r_rot, t_min, t_max, irc))
		return false;

	auto p = irc.p;
	auto normal = irc.normal;


	if (axis.x >= 1) {
		p[1] = cos_theta * irc.p[1] - sin_theta * irc.p[2];
		p[2] = sin_theta * irc.p[1] + cos_theta * irc.p[2];
		normal[1] = cos_theta * irc.normal[1] - sin_theta * irc.normal[2];
		normal[2] = sin_theta * irc.normal[1] + cos_theta * irc.normal[2];
	}
	else if (axis.y >= 1) {
		p[0] = cos_theta * irc.p[0] + sin_theta * irc.p[2];
		p[2] = -sin_theta * irc.p[0] + cos_theta * irc.p[2];
		normal[0] = cos_theta * irc.normal[0] + sin_theta * irc.normal[2];
		normal[2] = -sin_theta * irc.normal[0] + cos_theta * irc.normal[2];
	}
	else {
		p[0] = cos_theta * irc.p[0] - sin_theta * irc.p[1];
		p[1] = sin_theta * irc.p[0] + cos_theta * irc.p[1];
		normal[0] = cos_theta * irc.normal[0] - sin_theta * irc.normal[1];
		normal[1] = sin_theta * irc.normal[0] + cos_theta * irc.normal[1];
	}

	irc.p = p;
	irc.set_face_normal(r_rot, normal);

	return true;
}


using intersect_record = IntersectRecord;
using IntersectionList = IntersectList;
//using World = IntersectList;