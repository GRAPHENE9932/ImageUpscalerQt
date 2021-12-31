/*
 * ImageUpscalerQt - FSRCNN task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include "Task.h"
#include "TaskDesc.h"

struct TaskFSRCNN : public Task {
public:
	TaskFSRCNNDesc desc;

	TaskFSRCNN(TaskFSRCNNDesc desc);

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
