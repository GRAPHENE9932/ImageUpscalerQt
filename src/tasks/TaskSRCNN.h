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

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;
};
