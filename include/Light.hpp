#pragma once
#include <types.hpp>
#include <Ray.hpp>
#include <Intersect.hpp>

enum typelight {point, direction, spot};

class Light
{
public:
	typelight tp;
	point3 pos;
	color lightcolor;
	Light(const point3 posistion, const color lightcol, const typelight tplight = direction) : pos(posistion), lightcolor(lightcol), tp(tplight) {}
	static void Fresnel(const vec3& dir, const vec3& normal, const double ior, double& kr);
};

void Light::Fresnel(const vec3& dir, const vec3& normal, const double ior, double& kr)
{
    double cosi = glm::clamp(glm::dot(dir, normal) , -1.0, 1.0);
    double etai = 1, etat = ior;
    if (cosi > 0.0) 
        std::swap(etai, etat);
    // Compute sini using Snell's law
    double sint = etai / etat * sqrtf(std::max(0.0, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1.0) 
        kr = 1.0;
    else {
        double cost = sqrtf(std::max(0.0, 1 - sint * sint));
        cosi = fabsf(cosi);
        double Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        double Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
}