/*
 * ImageUpscalerQt - abstract task header
 * SPDX-FileCopyrightText: 2021-2022 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <string>

#include <OpenImageIO/imagebuf.h>
#include <QString>

#include "TaskDesc.hpp"

class Task {
public:
	bool cancel_requested = false;

	virtual float progress() const { return 0; };
	virtual OIIO::ImageBuf do_task(const OIIO::ImageBuf input, std::function<void()> cancelled) = 0;
	virtual const TaskDesc* get_desc() const = 0;
};
