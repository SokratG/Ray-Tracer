#pragma once
#include <types.hpp>


struct Screen
{
public:
	const double aspectratio = 3.0 / 2.0; //16.0 / 9.0;
	const lint screenwidth = 1024;
	const lint screenheight = static_cast<lint>(screenwidth / aspectratio); // 480
	const size_t num_ch = 3;
	const double viewportheight = 2.0; // viewport height 2 unit
	const double viewportwidth = viewportheight * aspectratio;// viewport wdith with respect aspect ratio
	const color backgroundcolor = color(0.5, 0.7, 1.0);
	const double gammacorrection = 2.0;
};