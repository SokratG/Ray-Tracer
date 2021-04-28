#pragma once
#include <utility.hpp>
#include <Image.hpp>

class Texture
{
public:
	virtual color value(double u, double v, const point3& p) const = 0;
};


class SolidColor : public Texture
{
public:
	SolidColor() : color_value(0.0) {}
	SolidColor(const color c) : color_value(c) {}

	SolidColor(const double r, const double g, const double b) :
				SolidColor(color(r, g, b)) {}

	virtual color value(double u, double v, const point3& p) const override {
		return color_value;
	}

private:
	color color_value;
};



/*
	Checker Texture - alternation of rectangle colors
*/

class CheckerTexture : public Texture
{
public:
	shared_ptr<Texture> odd_rect;
	shared_ptr<Texture> even_rect;
	double freq = 0.0;
public:
	CheckerTexture() {}
	CheckerTexture(shared_ptr<Texture> even, shared_ptr<Texture> odd, const double fr = 10.0) : even_rect(even), odd_rect(odd), freq(fr) {}
	CheckerTexture(const color c1, const color c2, const double fr = 10.0) :
		even_rect(make_shared<SolidColor>(c1)), odd_rect(make_shared<SolidColor>(c2)), freq(fr) {}
public:
	double frequency() const { return freq; }
	void set_frequency(double fr) { freq = fr; }
	virtual color value(double u, double v, const point3& p) const override;
};

color CheckerTexture::value(double u, double v, const point3& p) const
{
	auto sines = sin(freq * p.x) * sin(freq * p.y) * sin(freq * p.z);
	if (sines < 0)
		return odd_rect->value(u, v, p);
	else
		return even_rect->value(u, v, p);
}



class PerlinTexture : public Texture
{
public:
	PerlinTexture(const double scale=1.0) : ranvec(count_pts), freq(scale){
		for (int i = 0; i < PerlinTexture::count_pts; ++i) {
			ranvec[i] = random_unit_vector(-1.0, 1.0);
		}

		perm_x = perlin_generate_perm();
		perm_y = perlin_generate_perm();
		perm_z = perlin_generate_perm();
	}
/*
	virtual color value(double u, double v, const point3& p) const override {
		return color(1, 1, 1) * 0.5 * (1.0 + noise(freq * p));
	}
*/
	virtual color value(double u, double v, const point3& p) const override {
		/* return color(1, 1, 1) * turb(freq * p); */
		// with phase adjusting
		return color(1, 1, 1) * 0.5 * (1 + sin(freq * p.z + 10 * turb(p)));
	}

protected:
	double noise(const point3& p) const {
		auto u = p.x - floor(p.x);
		auto v = p.y - floor(p.y);
		auto w = p.z - floor(p.z);

		auto i = static_cast<int>(floor(p.x));
		auto j = static_cast<int>(floor(p.y));
		auto k = static_cast<int>(floor(p.z));
		vec3 area[2][2][2];

		for (int di = 0; di < 2; di++)
			for (int dj = 0; dj < 2; dj++)
				for (int dk = 0; dk < 2; dk++)
					area[di][dj][dk] = ranvec[
							perm_x[(i + di) & 255] ^
							perm_y[(j + dj) & 255] ^
							perm_z[(k + dk) & 255] ];
					
		return perlin_interp(area, u, v, w);
	}


	double turb(const point3& p, const int depth = 7) const {
		auto accum = 0.0;
		auto temp_p = p;
		auto weight = 1.0;

		for (int i = 0; i < depth; i++) {
			accum += weight * noise(temp_p);
			weight *= 0.5;
			temp_p *= 2;
		}

		return fabs(accum);
	}

private:
	static const int count_pts = 256;
	std::vector<vec3> ranvec;
	std::vector<int> perm_x;
	std::vector<int> perm_y;
	std::vector<int> perm_z;
	double freq;

	double perlin_interp(const vec3 area[2][2][2], const double u, const double v, const double w) const {
		/* Hermitian cubic(smoothing) */
		auto uu = u * u * (3 - 2 * u);
		auto vv = v * v * (3 - 2 * v);
		auto ww = w * w * (3 - 2 * w);

		auto accum = 0.0;
		for (int i = 0; i < 2; i++)
			for (int j = 0; j < 2; j++)
				for (int k = 0; k < 2; k++) {
					vec3 weight_v(u - i, v - j, w - k);
					accum += (i * uu + (1 - i) * (1 - uu))
						* (j * vv + (1 - j) * (1 - vv))
						* (k * ww + (1 - k) * (1 - ww))
						* glm::dot(area[i][j][k], weight_v);
				}

		return accum;
	}

	static std::vector<int> perlin_generate_perm() {
		std::vector<int> perm(count_pts);
		for (int i = 0; i < PerlinTexture::count_pts; i++)
			perm[i] = i;
		permute(perm, PerlinTexture::count_pts);
		return perm;
	}

	static void permute(std::vector<int>& p, int n) {
		assert(!p.empty());
		for (auto i = n - 1; i > 0; --i) {
			int target = random_int(0, i);
			int temp = p[i];
			p[i] = p[target];
			p[target] = temp;
		}
	}
};




class ImageTexture : public Texture
{
private:
	const color def_color;
public:
	ImageTexture(const std::string& filepath) : 
		def_color(0.0, 1.0, 1.0), img_tex(filepath), width(0), height(0) {
		width = img_tex.get_width();
		height = img_tex.get_height();
	}
	virtual color value(double u, double v, const point3& p) const override {

		u = glm::clamp(u, 0.0, 1.0);
		v = 1.0 - glm::clamp(v, 0.0, 1.0);
		auto i = static_cast<int>(u * width);
		auto j = static_cast<int>(v * height);
		i = glm::clamp(i, 0, width - 1);
		j = glm::clamp(j, 0, width - 1);

		color pixel;
		auto is_read = img_tex.get_color(i, j, pixel);
		if (!is_read)
			return def_color;

		constexpr auto color_scale = 1.0 / 255.0;

		return color(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
	}
	int get_width() const { return width; }
	int get_height() const { return height; }

private:
	Image img_tex;
	int width, height;
};