#include <sstream>

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

std::string TaskSRCNN::to_string(unsigned short index) {
	std::stringstream ss;
	//1: use SRCNN 5-1-9 64-32
	ss << (index + 1) << ": use SRCNN " <<
	+kernels[0] << '-' << +kernels[1] << '-' << +kernels[2] << ' ' <<
	+channels[0] << '-' << +channels[1];

	return ss.str();
}
