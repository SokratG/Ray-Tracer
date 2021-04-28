#pragma once
#include <utility.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


enum image_type { image_png, image_jpg };

class Image
{
private:
	lint width;
	lint height;
	size_t num_ch;
	size_t buffer_size = 0;
	byte* framebuffer = nullptr;

public:
	Image(const lint img_width, const lint img_height, const size_t num_channel = 3) : 
		width(img_width), height(img_height), num_ch(num_channel)
	{
		assert(num_channel == 3);
		buffer_size = img_width * img_height * num_channel;
		framebuffer = new byte[buffer_size]{ 0 };
	}

	Image(const std::string& filepath, const int components_per_pixel = 0) {
		// stbi_set_flip_vertically_on_load(true);
		auto texwidth = 0; auto texheight = 0; auto nrComponents = 0;
		uchar* data = nullptr;
		data = stbi_load(filepath.c_str(), &texwidth, &texheight, &nrComponents, components_per_pixel);
		if (data) {
			width = texwidth; height = texheight; num_ch = nrComponents;
			buffer_size = texwidth * texheight * nrComponents;
			framebuffer = new byte[buffer_size]{ 0 };
			std::memcpy(framebuffer, data, buffer_size);
		}
		else
		{
			stbi_image_free(data);
			assert(false && "Texture failed to load at path");
		}
		stbi_image_free(data);
	}

	~Image() { if (framebuffer) delete[] framebuffer; }
	lint get_width() const { return width; }
	lint get_height() const { return height; }
	size_t get_num_ch() const { return num_ch; }
	size_t get_buff_size() const { return buffer_size; }
	byte* get_framebuffer_ptr() const { if (framebuffer) return framebuffer; else return nullptr; }



	void read_from_file(const std::string& filepath, const int components_per_pixel = 0) {

		if (framebuffer) {
			std::memset(framebuffer, 0, buffer_size);
			delete[] framebuffer;
			framebuffer = nullptr;
		}

		// stbi_set_flip_vertically_on_load(true);
		auto texwidth = 0; auto texheight = 0; auto nrComponents = 0;
		uchar* data = nullptr;
		data = stbi_load(filepath.c_str(), &texwidth, &texheight, &nrComponents, components_per_pixel);
		if (data) {
			width = texwidth; height = texheight; num_ch = nrComponents;
			buffer_size = texwidth * texheight * nrComponents;
			framebuffer = new byte[buffer_size]{ 0 };
			std::memcpy(framebuffer, data, buffer_size);
		}
		else
		{
			stbi_image_free(data);
			assert(false && "Texture failed to load at path");
		}
		stbi_image_free(data);
	}


	bool get_color(const lint x, const lint y, color& pixel) const {

		if (framebuffer == nullptr)
			return false;
		if (num_ch == 1) {
			pixel[0] = framebuffer[num_ch * (x + y * width)];
		}
		else if (num_ch == 3) {
			pixel[0] = framebuffer[num_ch * (x + y * width)];
			pixel[1] = framebuffer[num_ch * (x + y * width) + 1];
			pixel[2] = framebuffer[num_ch * (x + y * width) + 2];
		}
		else { /* with alpha */
			pixel[0] = framebuffer[num_ch * (x + y * width)];
			pixel[1] = framebuffer[num_ch * (x + y * width) + 1];
			pixel[2] = framebuffer[num_ch * (x + y * width) + 2];
			pixel[3] = framebuffer[num_ch * (x + y * width) + 3];
		}

		return true;
	}


	bool get_color(const lint base, color& pixel) const {

		if (framebuffer == nullptr)
			return false;
		if (num_ch == 1) {
			pixel[0] = framebuffer[num_ch * base];
		}
		else if (num_ch == 3) {
			pixel[0] = framebuffer[num_ch * base];
			pixel[1] = framebuffer[num_ch * base + 1];
			pixel[2] = framebuffer[num_ch * base + 2];
		}
		else { /* with alpha */
			pixel[0] = framebuffer[num_ch * base];
			pixel[1] = framebuffer[num_ch * base + 1];
			pixel[2] = framebuffer[num_ch * base + 2];
			pixel[3] = framebuffer[num_ch * base + 3];
		}
		
		return true;
	}

	void set_color(const lint x, const lint y, const color pixel) {

		assert(framebuffer != nullptr);
		if (num_ch == 1) {
			framebuffer[num_ch * (x + y * width)] = pixel[0];
		}
		else if (num_ch == 3) {
			framebuffer[num_ch * (x + y * width)] = pixel[0];
			framebuffer[num_ch * (x + y * width) + 1] = pixel[1];
			framebuffer[num_ch * (x + y * width) + 2] = pixel[2];
		}
		else { /* with alpha */
			framebuffer[num_ch * (x + y * width)] = pixel[0];
			framebuffer[num_ch * (x + y * width) + 1] = pixel[1];
			framebuffer[num_ch * (x + y * width) + 2] = pixel[2];
			framebuffer[num_ch * (x + y * width) + 3] = pixel[3];
		}
		
	}

	void set_color(const lint base, const color pixel) {

		assert(framebuffer != nullptr);

		if (num_ch == 1) {
			framebuffer[num_ch * base] = pixel[0];
		}
		else if (num_ch == 3) {
			framebuffer[num_ch * base] = pixel[0];
			framebuffer[num_ch * base + 1] = pixel[1];
			framebuffer[num_ch * base + 2] = pixel[2];
		}
		else { /* with alpha */
			framebuffer[num_ch * base] = pixel[0];
			framebuffer[num_ch * base + 1] = pixel[1];
			framebuffer[num_ch * base + 2] = pixel[2];
			framebuffer[num_ch * base + 3] = pixel[3];
		}
	}
	

	bool save_image(const std::string& filename, const image_type img_type) const {
		assert(width > 0 && height > 0 && num_ch > 0);
		assert(framebuffer != nullptr);
		assert(!filename.empty());

		constexpr int quality = 100; // [1, 100]

		switch (img_type)
		{
		case image_type::image_png:
			stbi_write_png(filename.c_str(), width, height, num_ch, framebuffer, width * num_ch);
			break;
		case image_type::image_jpg:
			stbi_write_jpg(filename.c_str(), width, height, num_ch, framebuffer, quality);
			break;
		default:
			return false;
			break;
		}
		return true;
	}

	static bool save_framebuffer(const std::string& filename, const lint width, const lint height, const lint num_ch, const image_type img_type, const byte* data)
	{
		assert(width > 0 && height > 0 && num_ch > 0);
		assert(data != nullptr);
		assert(!filename.empty());

		constexpr int quality = 100; // [1, 100]

		switch (img_type)
		{
		case image_type::image_png:
			stbi_write_png(filename.c_str(), width, height, num_ch, data, width * num_ch);
			break;
		case image_type::image_jpg:
			stbi_write_jpg(filename.c_str(), width, height, num_ch, data, quality);
			break;
		default:
			return false;
			break;
		}
		return true;
	}
};