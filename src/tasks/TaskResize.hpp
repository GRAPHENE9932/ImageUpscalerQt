/*
 * ImageUpscalerQt - resize task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Task.hpp"
#include "TaskDesc.hpp"

class TaskResize : public Task {
public:
	TaskResizeDesc desc;

	explicit TaskResize(TaskResizeDesc desc);

	OIIO::ImageBuf do_task(OIIO::ImageBuf input, std::function<void()> canceled) override;

	const TaskDesc* get_desc() const override;
};
