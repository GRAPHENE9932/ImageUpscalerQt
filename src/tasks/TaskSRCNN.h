/*
 * ImageUpscalerQt - SRCNN task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include "Task.h"
#include "TaskDesc.h"

struct TaskSRCNN : public Task {
public:
	TaskSRCNNDesc desc;

	TaskSRCNN(TaskSRCNNDesc desc);

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
