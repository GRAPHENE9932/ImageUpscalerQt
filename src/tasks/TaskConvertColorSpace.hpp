/*
 * ImageUpscalerQt - convert color space task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Task.hpp"
#include "TaskDesc.hpp"

class TaskConvertColorSpace : public Task {
public:
	TaskConvertColorSpaceDesc desc;

	TaskConvertColorSpace(TaskConvertColorSpaceDesc desc);

	float progress() const override;

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;

	const TaskDesc* get_desc() const override;

private:
	float progress_val;
};
