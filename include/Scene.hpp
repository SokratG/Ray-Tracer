#pragma once
#include <camera.hpp>
#include <utility.hpp>
#include <sphere.hpp>
#include <triangle.hpp>
#include <box.hpp>
#include <volumetric.hpp>
#include <option.hpp>
#include <Material.hpp>
#include <Light.hpp>
#include <Image.hpp>
#ifdef _USE_THREAD
#include <ThreadPool.h>
#endif
#include <exception>
#include <cassert>



class Scene
{
public:
	Scene() {}
	bool init(const shared_ptr<Screen>& scn, const shared_ptr<Camera>& cam, const RayTracerOption& option);
	void render(Image& image, const IntersectList& world);
#ifdef _USE_THREAD
	void thread_render(Image& image, const IntersectList& world);
#endif

private:
	void draw_pixel(const IntersectList& world, const lint i, const lint j, color& pixel);
	color ray_color(const Ray& ray, const IntersectList& world, lint depth);
private:
	shared_ptr<Camera> camera;
	shared_ptr<Screen> screen;
	color backcolor = blackcolor;
	lint img_width = 0;
	lint img_height = 0;
	lint sample_per_pixel = 0;
	int maxdepth = 0;
	double gammacorrection = 1.0;
	bool isInit = false;
};

bool Scene::init(const shared_ptr<Screen>& scn, const shared_ptr<Camera>& cam, const RayTracerOption& option)
{
	assert(option.maxdepth > 0);
	assert(scn->gammacorrection > 0);

	if (isInit)
		return false;

	screen = scn;
	camera = cam;
	img_width = screen->screenwidth;
	img_height = screen->screenheight;
	sample_per_pixel = option.sample_per_pixel;
	this->maxdepth = option.maxdepth;
	gammacorrection = scn->gammacorrection;
	backcolor = screen->backgroundcolor;


	isInit = true;
	return isInit;
}

void Scene::render(Image& image, const IntersectList& world)
{
	assert(isInit == true);

	lint base = 0;
	color pixel{ 0 };
	for (lint j = img_height - 1; j >= 0; --j) {
		for (lint i = 0; i < img_width; ++i) {
			draw_pixel(world, i, j, pixel);
			image.set_color(base, pixel);
			base += 1;
		}
	}

}


void Scene::draw_pixel(const IntersectList& world, const lint i, const lint j, color& pixel)
{
	lint width = camera->get_screen_width() - 1;
	lint height = camera->get_screen_height() - 1;
	color pixel_color(0, 0, 0);

	// Path tracing sampling method
	for (lint s = 0; s < sample_per_pixel; ++s) {
		auto u = (i + random_double()) / width;
		auto v = (j + random_double()) / height;
		Ray ray = camera->get_ray(u, v);
		pixel_color += ray_color(ray, world, maxdepth);
	}
	AA_RGBPixel(pixel, pixel_color, sample_per_pixel, gammacorrection);
}

color Scene::ray_color(const Ray& ray, const IntersectList& world, lint depth)
{
	// If we've exceeded the ray bounce limit, no more light is gathered.
	if (depth <= 0)
		return backcolor;  //screen->backgroundcolor

	IntersectRecord irc;
	if (!world.intersect(ray, 0.001, infinity, irc)) 
		return backcolor;
	
	Ray scattered;
	color attenuation(0.0);
	color emitted = irc.material->emitted(irc.uv.x, irc.uv.y, irc.p);
	if (!irc.material->scatter(ray, irc, attenuation, scattered)) 
		return emitted;
	
	return emitted + attenuation * ray_color(scattered, world, depth - 1);

#ifdef NO
	vec3 unit_dir = glm::normalize(ray.direction());
	auto t = 0.5 * (unit_dir.y + 1.0); // blend respect y position
	return (1.0 - t) * whitecolor + t * backcolor; // final background blend color
#endif
}



#ifdef _USE_THREAD
void Scene::thread_render(Image& image, const IntersectList& world)
{
	assert(isInit == true);
	// srand(time(NULL));

	lint thread_num = std::thread::hardware_concurrency() - 1;
	lint total_block = img_width;
	lint block = img_width / thread_num;

	{ /* Parallel block start */
		lint base = 0;
		ThreadPool thp(thread_num);
		size_t s = 0;
		while (total_block > 0) {
			if ((total_block - block) > 0) {

				thp.enqueue([=](Image& image, const IntersectList& world) {
					lint offset = base;
					color pixel{ 0 };
					for (lint j = img_height - 1; j >= 0; --j) {
						for (lint i = base; i < base + block; ++i) {
							draw_pixel(world, i, j, pixel);
							image.set_color(offset, pixel);
							offset += 1;
						}
						offset += img_width - block;
					}
					}, std::ref(image), std::ref(world));

			}
			else {

				thp.enqueue([=](Image& image, const IntersectList& world) {
					lint offset = base;
					color pixel{ 0 };
					for (lint j = img_height - 1; j >= 0; --j) {
						for (lint i = base; i < base + total_block; ++i) {
							draw_pixel(world, i, j, pixel);
							image.set_color(offset, pixel);
							offset += 1;
						}
						offset += img_width - total_block;
					}
					}, std::ref(image), std::ref(world));

			}
			base += block;
			total_block -= block;
		}
	} /* Parallel block end */

}
#endif