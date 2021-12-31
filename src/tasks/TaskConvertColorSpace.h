/*
 * ImageUpscalerQt - convert color space task header
 * SPDX-FileCopyrightText: 2021 Artem Kliminskyi, artemklim50@gmail.com
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include "Task.h"

enum class ColorSpaceConversion : unsigned char {
	rgb_to_ycbcr, ycbcr_to_rgb, rgb_to_ycocg, ycocg_to_rgb
};

/// Color space names for the user.
const QString COLOR_SPACE_CONVERSION_NAMES[4] = {
	"RGB to YCbCr", "YCbCr to RGB", "RGB to YCoCg", "YCoCg to RGB"
};

class TaskConvertColorSpace : public Task {
public:
	ColorSpaceConversion color_space_conversion;

	TaskConvertColorSpace();
	TaskConvertColorSpace(ColorSpaceConversion color_space_conversion);

	/// "1: convert from RGB to YCbCr".
	QString to_string(unsigned short index) const override;
	/// "convert from RGB to YCbCr".
	QString to_string() const override;
	float progress() const override;

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;

private:
	float progress_val;
};
