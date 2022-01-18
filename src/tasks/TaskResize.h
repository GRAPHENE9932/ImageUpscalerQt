/*
 * ImageUpscalerQt - resize task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Task.h"
#include "TaskDesc.h"

class TaskResize : public Task {
public:
	TaskResizeDesc desc;

	TaskResize(TaskResizeDesc desc);

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;
};
