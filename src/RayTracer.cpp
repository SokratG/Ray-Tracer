// RayTracer.cpp : Defines the entry point for the application.
//
#include <generate_scene.hpp>
#include <option.hpp>


shared_ptr<IntersectList> generate_world(const scene_type num_scene, CameraOption& cameraopt, shared_ptr<Screen>& screen)
{
	cameraopt.lookfrom = point3(13, 2, 3);
	cameraopt.lookat = point3(0, 0, 0);
	cameraopt.up = vec3(0, 1, 0);
	cameraopt.fovy = 20.0;
	switch (num_scene)
	{
	case RANDOM_SCENE: {
		cameraopt.aperture = 0.1;
		return generate_random_scene();
	}
	case FINAL_SCENE: {
		cameraopt.lookfrom = point3(478, 278, -600);
		cameraopt.lookat = point3(278, 278, 0);
		cameraopt.fovy = 40.0;
		screen->aspectratio = 1.0;
		screen->screenwidth = 800;
		screen->screenheight = static_cast<lint>(screen->screenwidth / screen->aspectratio);
		screen->backgroundcolor = blackcolor;
		return generate_final_scene();
	}
	default:
		break;
	}
	return make_shared<IntersectionList>();
}

#include <profile/timeprofile.hpp>
int main(int argc, char* argv[])	
{
	std::string outfn = "final_scene.png";

	// screen
	shared_ptr<Screen> screen = make_shared<Screen>();
	
	// Raytracer and Camera options;
	Option option;
	CameraOption cameraopt;
	
	// wolrd
	shared_ptr<IntersectList> world = generate_world(FINAL_SCENE, cameraopt, screen);

	// camera
	shared_ptr<Camera> camera = make_shared<Camera>(*screen, cameraopt, 0.0, 1.0);

	// image
	Image image(screen->screenwidth, screen->screenheight, screen->num_ch);

	// scene
	Scene scene;
	scene.init(screen, camera, option);
	{
		TimeProfile tp(true);
#ifdef _USE_THREAD
		scene.thread_render(image, *world);
#else
		scene.render(image, *world);
#endif
	}


	// save
	image.save_image(outfn, image_png);

	
	return 0;
}


