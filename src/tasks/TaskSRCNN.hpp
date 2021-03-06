/*
 * ImageUpscalerQt - SRCNN task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <array>

#include "Task.hpp"
#include "TaskDesc.hpp"

struct TaskSRCNN : public Task {
public:
	TaskSRCNNDesc desc;

	explicit TaskSRCNN(const TaskSRCNNDesc& desc);

	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input, std::function<void()> cancelled) override;

	const TaskDesc* get_desc() const override;

private:
	long long blocks_amount = 0;
	long long blocks_processed = 0;
};
