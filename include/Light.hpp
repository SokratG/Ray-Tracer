#pragma once
#include <types.hpp>
#include <Ray.hpp>
#include <Intersect.hpp>

enum typelight {point_light, direction_light, spot_light};


class ILight
{
protected:
	color lightcolor;
public:
	typelight tp;
public:
	ILight(const color lightcol, const typelight tplight) : lightcolor(lightcol), tp(tplight) {}
	/*
		Generate a ray from a given point to the light source.
	*/
	virtual Ray get_ray_light(const point3& surfpoint) const = 0;

	/*
		Get the distance from a world point to the light source.
	*/
	virtual double get_distance(const point3& position) const = 0;

	virtual color get_color_at(const point3& position) const = 0;
	void  set_color(const color& light_color) { lightcolor = light_color; }
};

