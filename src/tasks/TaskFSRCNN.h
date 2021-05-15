#pragma once

#include <array>

#include "Task.h"

struct TaskFSRCNN : public Task {
	std::array<unsigned char, 4> kernels;
	std::array<unsigned char, 4> paddings;
	std::array<unsigned char, 3> channels;

	TaskFSRCNN();
	TaskFSRCNN(std::array<unsigned char, 4> kernels, std::array<unsigned char, 4> paddings,
			   std::array<unsigned char, 3> channels);

	std::string to_string(unsigned short index) override;
};
