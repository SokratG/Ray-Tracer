#pragma once
#include <utility.hpp>

struct IntersectRecord;


enum material_type{ DIFFUSE_AND_GLOSSY, REFLECTION_AND_REFRACTION, REFLECTION };

struct color_pack
{
	color specular_color;
	/*color diffuse_color;*/ // use albedo instead
	color ambient_color;
	double glossiness;
	double reflectivity;
	double refractivity_index;
	color_pack(const color& specular = color(0.0), const color& ambient = color(0.0),
		const double glossiness_color = 0.0, const double reflectivity_color = 0.0, const double refractivity_index_color = 1.0) :
		specular_color(specular), glossiness(glossiness_color), ambient_color(ambient),
		reflectivity(reflectivity_color), refractivity_index(refractivity_index_color)
	{
		assert(reflectivity <= 1.0 && reflectivity >= 0.0);
	}
};

using color_data = color_pack;

class Material
{
private:
	/* use default value */
public:
	material_type mat_type;
	shared_ptr<color_data> color_property;
#pragma warning(push)
#pragma warning(disable : 26812)
	Material(material_type mt, const shared_ptr<color_data> color_data = nullptr) : mat_type(mt), color_property(color_data) {}
#pragma warning(pop)
public:
	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const = 0;
};



// diffuse case
class Lambertian : public Material
{
public:
	color albedo;
public:
	Lambertian(const color& c) : Material(DIFFUSE_AND_GLOSSY), albedo(c) {}
	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override
	{
		auto scattered_dir = irc.normal + random_unit_vector();

		// catch degenerate scatter direction
		if (near_zero(scattered_dir))
			scattered_dir = irc.normal;

		scattered = Ray(irc.p, scattered_dir);
		attenuation = albedo;
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
	Metal(const color& c, const double fuzz) : Material(REFLECTION), albedo(c), fuzzier(fuzz < 1.0 ? fuzz : 1.0) {}
	virtual bool scatter(const Ray& ray, const IntersectRecord& irc, color& attenuation, Ray& scattered) const override
	{
		vec3 reflected = glm::reflect(glm::normalize(ray.direction()), irc.normal);
		scattered = Ray(irc.p, reflected + fuzzier * random_unit_in_sphere());
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
	Dielectric(const double index_of_refraction) : Material(REFLECTION_AND_REFRACTION), ir(index_of_refraction) {}

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

		scattered = Ray(irc.p, dir);
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