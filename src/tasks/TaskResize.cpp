#include <sstream>

#include "TaskResize.h"

TaskResize::TaskResize() {
	this->task_kind = TaskKind::resize;
}

TaskResize::TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation) {
	this->task_kind = TaskKind::resize;

	this->x_size = x_size;
	this->y_size = y_size;
	this->interpolation = interpolation;
}

std::string TaskResize::to_string(unsigned short index) {
	std::stringstream ss;
	//1: resize to 1920x1080 | Bilinear
	ss << (index + 1) << ": resize to " << x_size << 'x' << y_size << " | " <<
	INTERPOLATION_NAMES[(unsigned char)interpolation];

	return ss.str();
}
