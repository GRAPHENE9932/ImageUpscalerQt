#include "TaskSRCNN.h"

TaskSRCNN::TaskSRCNN() {
	this->task_kind = TaskKind::srcnn;
}

TaskSRCNN::TaskSRCNN(std::array<unsigned char, 3> kernels, std::array<unsigned char, 3> paddings,
					 std::array<unsigned char, 2> ch_n) {
	this->task_kind = TaskKind::srcnn;

	this->kernels = kernels;
	this->paddings = paddings;
	this->channels = ch_n;
}
