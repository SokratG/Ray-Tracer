#pragma once
#include <Scene.hpp>

enum scene_type {
	RANDOM_SCENE = 0, FINAL_SCENE
};


shared_ptr<IntersectList> generate_random_scene()
{
	shared_ptr<IntersectList> world = make_shared<IntersectionList>();
	auto ground_material = make_shared<Lambertian>(color(0.5, 0.5, 0.5));

	auto checker = make_shared<CheckerTexture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

	world->add(make_shared<Sphere>(point3(0, -1000.0, 0), 1000, make_shared<Lambertian>(checker)));

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
					auto center_end = center + vec3(0, random_double(0, 0.5), 0); // move to end point animation sphere
					world->add(make_shared<AnimationSphere>(center, center_end, 0.0, 1.0, 0.2, sphere_material)); /* time [0.0, 1.0] */
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

	auto material1 = make_shared<Dielectric>(1.5);
	auto material2 = make_shared<Metal>(color(0.7, 0.6, 0.5), 0.1);

	world->add(make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));
	world->add(make_shared<Triangle>(point3(1, 0, 0), point3(1, 2, 0), point3(4, 0, 0), material2));

	return make_shared<IntersectList>(make_shared<BVH_Node>(*world, 0.0, 1.0));
}


shared_ptr<IntersectList> generate_final_scene()
{
	shared_ptr<IntersectList> boxes1 = make_shared<IntersectionList>();
	auto ground = make_shared<Lambertian>(color(0.48, 0.83, 0.53));

	constexpr int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++) {
		for (int j = 0; j < boxes_per_side; j++) {
			auto w = 100.0;
			auto x0 = -1000.0 + i * w;
			auto z0 = -1000.0 + j * w;
			auto y0 = 0.0;
			auto x1 = x0 + w;
			auto y1 = random_double(1, 101);
			auto z1 = z0 + w;

			boxes1->add(make_shared<Box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	}

	shared_ptr<IntersectList> world = make_shared<IntersectionList>();

	world->add(make_shared<BVH_Node>(*boxes1, 0.0, 1.0));

	auto light = make_shared<DiffuseLight>(color(7, 7, 7));
	world->add(make_shared<xzRect>(123, 423, 147, 412, 554, light));

	auto glasstri = make_shared<Dielectric>(5.0);
	world->add(make_shared<Triangle>(point3(123, 100, 50), point3(147, 120, 50), point3(250, 120, 50), glasstri));
	world->add(make_shared<Triangle>(point3(140, 150, 50), point3(147, 120, 50), point3(250, 120, 50), glasstri));

	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + vec3(30, 0, 0);
	auto moving_sphere_material = make_shared<Lambertian>(color(0.7, 0.3, 0.1));
	world->add(make_shared<AnimationSphere>(center1, center2, 0, 1, 50, moving_sphere_material));

	
	world->add(make_shared<Sphere>(point3(260, 150, 45), 50, make_shared<Dielectric>(1.5)));
	world->add(make_shared<Sphere>(
		point3(0, 150, 145), 50, make_shared<Metal>(color(0.8, 0.8, 0.9), 1.0)
		));

	auto boundary = make_shared<Sphere>(point3(360, 150, 145), 70, make_shared<Dielectric>(1.5));
	world->add(boundary);
	world->add(make_shared<ConstantVolume>(boundary, 0.2, color(0.2, 0.4, 0.9)));
	boundary = make_shared<Sphere>(point3(0, 0, 0), 5000, make_shared<Dielectric>(1.5));
	world->add(make_shared<ConstantVolume>(boundary, 0.0001, color(1, 1, 1)));


	auto emat = make_shared<Lambertian>(make_shared<ImageTexture>("earthmap.jpg"));
	world->add(make_shared<Sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<PerlinTexture>(0.1);
	world->add(make_shared<Sphere>(point3(220, 280, 300), 80, make_shared<Lambertian>(pertext)));


	shared_ptr<IntersectList> boxes2 = make_shared<IntersectionList>();
	auto white = make_shared<Lambertian>(color(.73, .73, .73));
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxes2->add(make_shared<Sphere>(generate_random_vec(0, 165), 10, white));
	}
	
	world->add(make_shared<Translate>(
					make_shared<Rotate>(make_shared<BVH_Node>(*boxes2, 0.0, 1.0), vec3(0, 1, 0), 15),
			   vec3(-100, 270, 395)));
									  
	return world;
}