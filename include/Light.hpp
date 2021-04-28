#pragma once
#include <types.hpp>
#include <Intersect.hpp>
#include <Material.hpp>


class DiffuseLight : public Material
{
public:
	DiffuseLight(shared_ptr<Texture> tex) : emit(tex) {}
	DiffuseLight(const color c) : emit(make_shared<SolidColor>(c)) {}

	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override
	{
		return false;
	}
	
	virtual color emitted(double u, double v, const point3& p) const override {
		return emit->value(u, v, p);
	}
public:
	shared_ptr<Texture> emit;
};

