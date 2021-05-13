#pragma once

#include "Task.h"

enum class Interpolation : unsigned char {
	none = 0,
	box = 1,
	bilinear = 2,
	triangle = 3,
	gaussian = 4,
	catmull_rom = 5,
	blackman_harris = 6,
	sinc = 7,
	lanczos3 = 8,
	b_spline = 9,
	disk = 10,
	radial_lanczos3 = 11,
	mitchell = 12,
	cubic = 13,
	keys = 14,
	simon = 15,
	rifman = 16,
	nuke_lanczos6 = 17
};

struct TaskResize : public Task {
	unsigned int x_size = 0, y_size = 0;
	Interpolation interpolation = Interpolation::none;

	TaskResize();
	TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation);
};
