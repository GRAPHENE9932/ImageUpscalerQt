#include <sstream>

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

std::string TaskFSRCNN::to_string(unsigned short index) {
	std::stringstream ss;
	//1: use FSRCNN 3-1-3-4 64-32-32
	ss << (index + 1) << ": use FSRCNN " <<
	+kernels[0] << '-' << +kernels[1] << '-' << +kernels[2] << +kernels[3] << ' ' <<
	+channels[0] << '-' << +channels[1] << '-' << +channels[2];

	return ss.str();
}

OIIO::ImageBuf TaskFSRCNN::do_task(OIIO::ImageBuf input) {

}
