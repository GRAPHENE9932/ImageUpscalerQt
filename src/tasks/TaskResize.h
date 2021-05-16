#pragma once

#include "Task.h"

enum class Interpolation : unsigned char {
	none, bilinear, cubic, gaussian, sinc, box, triangle, lanczos3,
	catmull_rom, b_spline, mitchell
};

///Interpolation names for the user
const std::string INTERPOLATION_NAMES[18] = {
	"None", "Bilinear", "Cubic", "Gaussian", "Sinc", "Box", "Triangle", "Lanczos3",
	"Catmull-Rom", "B-spline", "Mitchell"
};

///Interpolation names for the OpenImageIO library.
///"none" and "bilinear" are not used.
const std::string INTERPOLATION_OIIO_NAMES[18] = {
	"none", "bilinear", "cubic", "gaussian", "sinc", "box", "triangle", "lanczos3",
	"catmull-rom", "bspline", "mitchell"
};

class TaskResize : public Task {
public:
	unsigned int x_size = 0, y_size = 0;
	Interpolation interpolation = Interpolation::none;

	TaskResize();
	TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation);

	std::string to_string(unsigned short index) const override;
	std::string to_string() const override;

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;
};
