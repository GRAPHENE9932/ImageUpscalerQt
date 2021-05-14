#pragma once

#include "Task.h"

enum class Interpolation : unsigned char {
	none, bilinear, cubic, gaussian, sinc, box, triangle, lanczos3,
	catmull_rom, b_spline, mitchell
};

const std::string INTERPOLATION_NAMES[18] = {
	"None", "Bilinear", "Cubic", "Gaussian", "Sinc", "Box", "Triangle", "Lanczos3",
	"Catmull-Rom", "B-spline", "Mitchell"
};

struct TaskResize : public Task {
	unsigned int x_size = 0, y_size = 0;
	Interpolation interpolation = Interpolation::none;

	TaskResize();
	TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation);

	std::string to_string(unsigned short index) override;
};
