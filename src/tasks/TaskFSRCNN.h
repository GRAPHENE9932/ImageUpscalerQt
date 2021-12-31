/*
 * ImageUpscalerQt - FSRCNN task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include "Task.h"

struct TaskFSRCNN : public Task {
public:
	std::vector<unsigned short> kernels;
	std::vector<unsigned short> paddings;
	std::vector<unsigned short> channels;
	/// Block size of the input image that will be splitted into blocks before the CNN.
	/// 0 if the input image have not to be splitted.
	unsigned int block_size;

	TaskFSRCNN();
	TaskFSRCNN(std::vector<unsigned short> kernels, std::vector<unsigned short> paddings,
			   std::vector<unsigned short> channels, unsigned int block_size);

	/// "1: use FSRCNN 3-1-3-4 64-32-32".
	QString to_string(unsigned short index) const override;
	/// "use FSRCNN 3-1-3-4 64-32-32".
	QString to_string() const override;
	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input) override;

private:
	long long blocks_amount;
	long long blocks_processed;
};
