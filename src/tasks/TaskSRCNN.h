#pragma once

#include <array>

#include "Task.h"

struct TaskSRCNN : public Task {
public:
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 3> paddings;
	std::array<unsigned short, 2> channels;

	TaskSRCNN();
	TaskSRCNN(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
			  std::array<unsigned short, 2> channels);

	///"1: use SRCNN 5-1-9 64-32"
	QString to_string(unsigned short index) const override;
	///"use SRCNN 5-1-9 64-32"
	QString to_string() const override;
	QString parameters_path() const;
	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input) override;

private:
	long long blocks_amount;
	long long blocks_processed;
};
