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
