#pragma once
#include <camera.hpp>
#include <utility.hpp>
#include <sphere.hpp>
#include <triangle.hpp>
#include <option.hpp>
#include <Material.hpp>
#include <Light.hpp>
#ifdef _USE_THREAD
#include <ThreadPool.h>
#endif
#ifdef _USE_CUDA
#include <cuda/cuda_raytracer.cuh>
#endif
#include <exception>
#include <cassert>

#define blackcolor color(0.0, 0.0, 0.0)
#define whitecolor color(1.0, 1.0, 1.0)

constexpr double bias = 0.00001;

class Scene
{
public:
	Scene() {}
	bool init(const std::vector<shared_ptr<Light>>& lights, const shared_ptr<Screen>& scn, const shared_ptr<Camera>& cam, const RayTracerOption& option);
	void render(byte* framebuffer, const IntersectList& world);
#ifdef _USE_THREAD
	void thread_render(byte* framebuffer, const IntersectList& world);
#endif
#ifdef _USE_CUDA
	void cuda_render(byte* framebuffer, const size_t buffer_size, const IntersectList& world);
#endif
private:
	void draw_pixel(const IntersectList& world, const lint i, const lint j, byte pixel[3]);
	color ray_color(const Ray& ray, const IntersectList& world, lint depth);
	
private:
	std::vector<shared_ptr<Light>> scene_lights;
private:
	shared_ptr<Camera> camera;
	shared_ptr<Screen> screen;
	lint img_width = 0;
	lint img_height = 0;
	lint sample_per_pixel = 0;
	int maxdepth = 0;
	double gammacorrection = 1.0;
	bool isInit = false;
};

bool Scene::init(const std::vector<shared_ptr<Light>>& lights, const shared_ptr<Screen>& scn, const shared_ptr<Camera>& cam, const RayTracerOption& option)
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

	if (!lights.empty())
		scene_lights = lights;

	isInit = true;
	return isInit;
}

void Scene::render(byte* framebuffer, const IntersectList& world)
{
	assert(isInit == true);
	assert(framebuffer != nullptr);

	lint base = 0;
	byte pixel[3]{ 0 };
	for (lint j = img_height - 1; j >= 0; --j) {
		for (lint i = 0; i < img_width; ++i) {
			draw_pixel(world, i, j, pixel);
			framebuffer[3 * base] = pixel[0];
			framebuffer[3 * base + 1] = pixel[1];
			framebuffer[3 * base + 2] = pixel[2];
			base += 1;
		}
	}
}


void Scene::draw_pixel(const IntersectList& world, const lint i, const lint j, byte pixel[3])
{
	lint width = camera->get_screen_width() - 1;
	lint height = camera->get_screen_height() - 1;
	color pixel_color(0, 0, 0);
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
		return blackcolor;

	IntersectRecord irc;
	if (world.intersect(ray, 0.001, infinity, irc)) {
		Ray scattered;
		color attenuation(0.0);
		if (irc.material->scatter(ray, irc, attenuation, scattered)) {
			return attenuation * ray_color(scattered, world, depth - 1);
		}
		return blackcolor;
	}
	vec3 unit_dir = glm::normalize(ray.direction());
	auto t = 0.5 * (unit_dir.y + 1.0); // blend respect y position
	return (1.0 - t) * whitecolor + t * screen->backgroundcolor; // final background blend color
}




#ifdef _USE_CUDA
void Scene::cuda_render(byte* framebuffer, const size_t buffer_size, const IntersectList& world)
{
	float gpuTime = 0.f;
	cudaEvent_t start, stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);

	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&gpuTime, start, stop);

	printf("Elapsed time: %.2f ms\n", gpuTime);

}
#endif

#ifdef _USE_THREAD
void Scene::thread_render(byte* framebuffer, const IntersectList& world)
{
	assert(isInit == true);
	assert(framebuffer != nullptr);
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

				thp.enqueue([=](byte* framebuffer, const IntersectList& world) {
					lint offset = base;
					byte pixel[3]{ 0 };
					for (lint j = img_height - 1; j >= 0; --j) {
						for (lint i = base; i < base + block; ++i) {
							draw_pixel(world, i, j, pixel);
							framebuffer[3 * offset] = pixel[0];
							framebuffer[3 * offset + 1] = pixel[1];
							framebuffer[3 * offset + 2] = pixel[2];
							offset += 1;
						}
						offset += img_width - block;
					}
					}, framebuffer, std::ref(world));

			}
			else {

				thp.enqueue([=](byte* framebuffer, const IntersectList& world) {
					lint offset = base;
					byte pixel[3]{ 0 };
					for (lint j = img_height - 1; j >= 0; --j) {
						for (lint i = base; i < base + total_block; ++i) {
							draw_pixel(world, i, j, pixel);
							framebuffer[3 * offset] = pixel[0];
							framebuffer[3 * offset + 1] = pixel[1];
							framebuffer[3 * offset + 2] = pixel[2];
							offset += 1;
						}
						offset += img_width - total_block;
					}
					}, framebuffer, std::ref(world));

			}
			base += block;
			total_block -= block;
		}
	} /* Parallel block end */

}
#endif