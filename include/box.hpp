#pragma once
#include <rect.hpp>



class Box : public IIntersect
{
public:
	Box() : sides(nullptr) {}
	Box(const point3& p0, const point3& p1, shared_ptr<Material> m_ptr);

	virtual bool intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& ir) const override;
	virtual bool bounding_box(double time0, double time1, AABB& output_box) const override {
		output_box = AABB(box_low, box_up);
		return true;
	}

public:
	point3 box_low;
	point3 box_up;
	shared_ptr<IntersectionList> sides;
};
 
Box::Box(const point3& p0, const point3& p1, shared_ptr<Material> m_ptr) : sides(make_shared<IntersectionList>())
{
	box_low = p0;
	box_up = p1;

	sides->add(make_shared<xyRect>(p0.x, p1.x, p0.y, p1.y, p1.z, m_ptr));
	sides->add(make_shared<xyRect>(p0.x, p1.x, p0.y, p1.y, p0.z, m_ptr));

	sides->add(make_shared<xzRect>(p0.x, p1.x, p0.z, p1.z, p1.y, m_ptr));
	sides->add(make_shared<xzRect>(p0.x, p1.x, p0.z, p1.z, p0.y, m_ptr));

	sides->add(make_shared<yzRect>(p0.y, p1.y, p0.z, p1.z, p1.x, m_ptr));
	sides->add(make_shared<yzRect>(p0.y, p1.y, p0.z, p1.z, p0.x, m_ptr));
}


bool Box::intersect(const Ray& ray, double t_min, double t_max, IntersectRecord& irc) const
{
	return sides->intersect(ray, t_min, t_max, irc);
}
