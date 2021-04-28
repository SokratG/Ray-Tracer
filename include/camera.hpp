#include <screen.hpp>
#include <Ray.hpp>
#include <utility.hpp>

struct CameraOption
{
	point3 lookfrom;
	point3 lookat;
	vec3 up;
	double fovy = 20.0;
	double aperture = 0.0; // depth of field: 0 - without blur and other value give a blur effect
	double focus_dist = 10.0;
};


class Camera
{
public:
	Camera(const Screen& screen, const CameraOption& cameraopt, const double time0=0.0, const double time1=0.0)
	{
		auto theta = glm::radians(cameraopt.fovy); // vertical field-of-view in degrees
		auto h = tan(theta / 2); 
		auto aspect_ratio = screen.aspectratio;
		auto viewport_height = screen.viewportheight * h;
		auto viewport_width = aspect_ratio * viewport_height;
		
		// calc camera position
		w = glm::normalize(cameraopt.lookfrom - cameraopt.lookat);
		u = glm::normalize(glm::cross(cameraopt.up, w));
		v = glm::cross(w, u);

		origin = cameraopt.lookfrom;
		horizontal = cameraopt.focus_dist * viewport_width * u; // horizontal NDC
		vertical = cameraopt.focus_dist * viewport_height * v; // vertical NDC
		lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - cameraopt.focus_dist * w;

		lens_radius = cameraopt.aperture / 2.0;

		// optional
		img_width = screen.screenwidth;
		img_height = screen.screenheight;

		// time
		tm0 = time0;
		tm1 = time1;
	}

	Ray get_ray(const double s, const double t) const {
		vec3 rd = lens_radius * random_unit_in_disk();
		vec3 offset = u * rd.x + v * rd.y;
		return Ray( origin + offset, 
					lower_left_corner + s * horizontal + t * vertical - origin - offset, 
					random_double(tm0, tm1));
	}

	lint get_screen_width() const { return img_width; }
	lint get_screen_height() const { return img_height; }

private:
	point3 origin;
	point3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	lint img_width;
	lint img_height;
	double lens_radius = 0.0;
	/* shutter open/close times - for motion blur */
	double tm0;
	double tm1;
};