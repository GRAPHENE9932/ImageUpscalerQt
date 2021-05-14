#include "TaskFSRCNN.h"

TaskFSRCNN::TaskFSRCNN() {
	this->task_kind = TaskKind::fsrcnn;
}

TaskFSRCNN::TaskFSRCNN(std::array<unsigned char, 4> kernels, std::array<unsigned char, 4> paddings,
					   std::array<unsigned char, 3> channels) {
	this->task_kind = TaskKind::fsrcnn;

	//Assign arrays
	this->kernels = kernels;

	this->paddings = paddings;

	this->channels = channels;
}

