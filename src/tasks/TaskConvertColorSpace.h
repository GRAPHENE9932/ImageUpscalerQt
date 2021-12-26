/*
 * ImageUpscalerQt convert color space task
 * Copyright (C) 2021  Artem Kliminskyi <artemklim50@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
