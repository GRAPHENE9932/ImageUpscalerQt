#pragma once

#include <array>

#include "Task.h"

struct TaskFSRCNN : public Task {
public:
	std::array<unsigned short, 4> kernels;
	std::array<unsigned short, 4> paddings;
	std::array<unsigned short, 3> channels;

	TaskFSRCNN();
	TaskFSRCNN(std::array<unsigned short, 4> kernels, std::array<unsigned short, 4> paddings,
			   std::array<unsigned short, 3> channels);

	std::string to_string(unsigned short index) const override;
	std::string parameters_path() const;
	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input) override;

private:
	long long blocks_amount;
	long long blocks_processed;
};
