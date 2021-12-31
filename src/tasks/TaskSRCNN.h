/*
 * ImageUpscalerQt - SRCNN task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include "Task.h"

struct TaskSRCNN : public Task {
public:
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 3> paddings;
	std::array<unsigned short, 4> channels;
	/// Block size of the input image that will be splitted into blocks before the CNN.
	/// 0 if the input image have not to be splitted.
	unsigned int block_size;

	TaskSRCNN();
	TaskSRCNN(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
			  std::array<unsigned short, 4> channels, unsigned int block_size);

	/// "1: use SRCNN 5-1-9 64-32".
	QString to_string(unsigned short index) const override;
	/// "use SRCNN 5-1-9 64-32".
	QString to_string() const override;

	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input) override;

private:
	long long blocks_amount;
	long long blocks_processed;
};
