#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <types.hpp>
#include <random>
#include <cassert>
#include <stdint.h>

using byte = uint8_t;

struct RT_Data
{
	int64_t width = 0;
	int64_t height = 0;
	int64_t sample_per_pixel = 0;
	double gammacorrection = 2.0;
	int64_t maxdepth = 0;
	color backgroundcolor;
};

__global__ void cu_draw_kernel(byte* cu_framebuffer, const RT_Data& rt_data);

inline double cu_random_double() {
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}

void cu_render(byte* framebuffer, const size_t buffer_size, 
				const color backcolor, const int64_t width, const int64_t height,
				int64_t sample_per_pixel, int64_t maxdepth, const double gammacorrection)
{
	byte* cu_framebuffer = nullptr;
	cudaError_t cudaStatus;
	
	RT_Data rt_data;
	rt_data.width = width;
	rt_data.height = height;
	rt_data.sample_per_pixel = sample_per_pixel;
	rt_data.gammacorrection = gammacorrection;
	rt_data.maxdepth = maxdepth;
	rt_data.backgroundcolor = backcolor;
	

	cudaStatus = cudaSetDevice(0); // pick first GPU device
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Error: set default GPU device: %s\n", cudaGetErrorString(cudaStatus));
		exit(EXIT_FAILURE);
	}


	cudaStatus = cudaMalloc(&cu_framebuffer, buffer_size * sizeof(byte));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Error: memory allocation for GPU device: %s\n", cudaGetErrorString(cudaStatus));
		exit(EXIT_FAILURE);
	}


	const uint32_t blockSize = rt_data.height;
	const uint32_t numBlocks = rt_data.width;
	
	// cu_render_kernel<<<dim3(numBlocks), dim3(blockSize)>>>(cu_framebuffer, world, rt_data);

	cudaStatus = cudaDeviceSynchronize();
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
		exit(EXIT_FAILURE);
	}


	cudaStatus = cudaMemcpy(framebuffer, cu_framebuffer, buffer_size * sizeof(byte), cudaMemcpyDeviceToHost);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Error: memory copy GPU-device to Host: %s\n", cudaGetErrorString(cudaStatus));
		exit(EXIT_FAILURE);
	}


	cudaStatus = cudaFree(cu_framebuffer);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "Error: memory deallocation for GPU device: %s\n", cudaGetErrorString(cudaStatus));
		exit(EXIT_FAILURE);
	}

	
}

__global__ void cu_AA_RGBPixel(byte pixels[3], const vec3& lightcolor, const size_t sample_per_pixel, const double gamma)
{
	assert(gamma > 0);
	auto r = lightcolor[0];
	auto g = lightcolor[1];
	auto b = lightcolor[2];

	auto scale = 1.0 / sample_per_pixel;
	// Divide the color by the number of samples and gamma-correct for gamma correction
	auto gammacorrection = 1.0 / gamma;
	r = pow((r * scale), gammacorrection);
	g = pow((g * scale), gammacorrection);
	b = pow((b * scale), gammacorrection);

	pixels[0] = static_cast<byte>(__saturatef(r) * 255);
	pixels[1] = static_cast<byte>(__saturatef(g) * 255);
	pixels[2] = static_cast<byte>(__saturatef(b) * 255);
}


