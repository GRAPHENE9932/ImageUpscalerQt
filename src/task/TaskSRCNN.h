#pragma once

#include <array>

#include "Task.h"

struct TaskSRCNN : public Task {
	std::array<unsigned char, 3> kernels;
	std::array<unsigned char, 3> paddings;
	std::array<unsigned char, 2> channels;

	TaskSRCNN();
	TaskSRCNN(std::array<unsigned char, 3> kernels, std::array<unsigned char, 3> paddings,
			  std::array<unsigned char, 2> channels);
};
