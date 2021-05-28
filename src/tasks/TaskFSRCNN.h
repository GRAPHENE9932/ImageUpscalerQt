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

	///"1: use FSRCNN 3-1-3-4 64-32-32"
	QString to_string(unsigned short index) const override;
	///"use FSRCNN 3-1-3-4 64-32-32"
	QString to_string() const override;
	QString parameters_path() const;
	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input) override;

private:
	long long blocks_amount;
	long long blocks_processed;
};
