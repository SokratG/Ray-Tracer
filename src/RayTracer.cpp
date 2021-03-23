// RayTracer.cpp : Defines the entry point for the application.
//
#include <Scene.hpp>
#include <option.hpp>

shared_ptr<IntersectList> generate_world()
{
	shared_ptr<IntersectList> world = make_shared<IntersectionList>();
	auto ground_material = make_shared<Lambertian>(color(0.5, 0.5, 0.5));
	world->add(make_shared<Sphere>(point3(0, -1000.0, 0), 1000, ground_material));
	/*
	for (int a = -3; a < 3; a++) {
		for (int b = -3; b < 3; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<Material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = random_color() * random_color();
					sphere_material = make_shared<Lambertian>(albedo);
					world->add(make_shared<Sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = random_color(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<Metal>(albedo, fuzz);
					world->add(make_shared<Sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<Dielectric>(1.5);
					world->add(make_shared<Sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}
	*/
	
	auto material1 = make_shared<Dielectric>(1.5);
	auto material2 = make_shared<Lambertian>(color(0.4, 0.2, 0.1));
	auto material3 = make_shared<Metal>(color(0.7, 0.6, 0.5), 0.1);
	
	/*
	world->add(make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));
	
	world->add(make_shared<Sphere>(point3(4, 1, 0), 1.0, material3));

	world->add(make_shared<Triangle>(point3(1, 0, 0), point3(1, 2, 0), point3(4, 0, 0), material3));
	*/

	// world->add(make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));
	// world->add(make_shared<Triangle>(point3(1, 0, 0), point3(1, 2, 0), point3(4, 0, 0), material3));

	
	return world;
}

std::vector<shared_ptr<Light>> generate_lights()
{
	std::vector<shared_ptr<Light>> lights;

	auto light1 = make_shared<Light>(point3(-5, -3, 0), color(1.0, 1.0, 1.0));

	lights.emplace_back(light1);

	return lights;
}


int main(int argc, char* argv[])	
{
	std::string outfn = "image.png";

	// screen
	shared_ptr<Screen> screen = make_shared<Screen>();
	
	// make a framebuffer
	byte* framebuffer = new byte[screen->screenwidth * screen->screenheight * screen->num_ch] {0};

	// Raytracer and Camera options;
	Option option;
	CameraOption cameraopt;
	cameraopt.lookfrom = point3(13, 2, 3);
	cameraopt.lookat = point3(0, 0, 0);
	cameraopt.up = vec3(0, 1, 0);
	cameraopt.focus_dist = 10.0;
	cameraopt.aperture = 0.1; 

	// camera
	shared_ptr<Camera> camera = make_shared<Camera>(*screen, cameraopt);

	// wolrd
	shared_ptr<IntersectList> world = generate_world();
	std::vector<shared_ptr<Light>> lights = generate_lights();

	// scene
	Scene scene;
	scene.init(lights, screen, camera, option);
	scene.render(framebuffer, *world);

	// save
	save_img(outfn, framebuffer, screen->screenwidth, screen->screenheight, screen->num_ch);

	delete[] framebuffer;

	return 0;
}


