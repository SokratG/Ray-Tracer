#pragma once
#include <utility.hpp>
#include <texture.hpp>

struct IntersectRecord;

#define blackcolor color(0.0, 0.0, 0.0)
#define whitecolor color(1.0, 1.0, 1.0)


class Material
{
private:
	/* use default value */
public:
#pragma warning(push)
#pragma warning(disable : 26812)
	Material() {}
#pragma warning(pop)
public:
	virtual color emitted(double u, double v, const point3& p) const { return blackcolor; }
	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const = 0;
};



// diffuse case
class Lambertian : public Material
{
public:
	shared_ptr<Texture> albedo;
public:

	Lambertian(const color& c) : albedo(make_shared<SolidColor>(c)) {}
	Lambertian(shared_ptr<Texture> tex) : albedo(tex) {}

	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override
	{
		auto scattered_dir = irc.normal + random_unit_vector();

		// catch degenerate scatter direction
		if (near_zero(scattered_dir))
			scattered_dir = irc.normal;

		scattered = Ray(irc.p, scattered_dir, ray.time());
		attenuation = albedo->value(irc.uv.x, irc.uv.y, irc.p);
		return true;
	}
};


// reflection case
class Metal : public Material
{
public:
	color albedo;
	double fuzzier;
public:
	Metal(const color& c, const double fuzz) : albedo(c), fuzzier(fuzz < 1.0 ? fuzz : 1.0) {}
	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override
	{
		vec3 reflected = glm::reflect(glm::normalize(ray.direction()), irc.normal);
		scattered = Ray(irc.p, reflected + fuzzier * random_unit_in_sphere(), ray.time());
		attenuation = albedo;
		return (glm::dot(scattered.direction(), irc.normal) > 0);
	}

};


// refraction case
class Dielectric : public Material
{
public:
	double ir; // Index of Refraction
public:
	Dielectric(const double index_of_refraction) : ir(index_of_refraction) {}

	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override
	{
		attenuation = color(1.0, 1.0, 1.0);
		double refraction_ratio = irc.front_face ? (1.0 / ir) : ir;
		vec3 unit_dir = glm::normalize(ray.direction());

		double cos_theta = fmin(glm::dot(-unit_dir, irc.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool is_reflect = (refraction_ratio * sin_theta) > 1.0;
		vec3 dir;
		// add Schlick's approximation
		if (is_reflect || reflectance(cos_theta, refraction_ratio) > random_double())
			dir = glm::reflect(unit_dir, irc.normal);
		else
			dir = glm::refract(unit_dir, irc.normal, refraction_ratio);

		scattered = Ray(irc.p, dir, ray.time());
		return true;
	}
private:
	static inline double reflectance(const double cosine, const double ref_idx) {
		// Use Schlick's approximation for reflectance.
		auto r0 = (1.0 - ref_idx) / (1.0 + ref_idx);
		r0 = r0 * r0;
		return r0 + (1.0 - r0) * glm::pow((1.0 - cosine), 5);
	}
};


class Isotropic : public Material
{
public:
	Isotropic(const color c) : albedo(make_shared<SolidColor>(c)) {}
	Isotropic(shared_ptr<Texture> a) : albedo(a) {}
	bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override {
		scattered = Ray(irc.p, random_unit_in_sphere(), ray.time());
		attenuation = albedo->value(irc.uv.x, irc.uv.y, irc.p);
		return true;
	}
public:
	shared_ptr<Texture> albedo;
};