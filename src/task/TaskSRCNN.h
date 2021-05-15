#pragma once

#include <array>

#include "Task.h"

struct TaskSRCNN : public Task {
public:
	std::array<unsigned char, 3> kernels;
	std::array<unsigned char, 3> paddings;
	std::array<unsigned char, 2> channels;

	TaskSRCNN();
	TaskSRCNN(std::array<unsigned char, 3> kernels, std::array<unsigned char, 3> paddings,
			  std::array<unsigned char, 2> channels);

	std::string to_string(unsigned short index) override;

	unsigned char* do_task(unsigned char* input, int width, int height, unsigned char ch_n) override;
};
