/*
 * ImageUpscalerQt resize task
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

enum class Interpolation : unsigned char {
	none, bilinear, cubic, gaussian, sinc, box, triangle, lanczos3,
	catmull_rom, b_spline, mitchell
};

///Interpolation names for the user
const QString INTERPOLATION_NAMES[11] = {
	"None", "Bilinear", "Cubic", "Gaussian", "Sinc", "Box", "Triangle", "Lanczos3",
	"Catmull-Rom", "B-spline", "Mitchell"
};

///Interpolation names for the OpenImageIO library.
///"none" and "bilinear" are not used.
const QString INTERPOLATION_OIIO_NAMES[11] = {
	"none", "bilinear", "cubic", "gaussian", "sinc", "box", "triangle", "lanczos3",
	"catmull-rom", "bspline", "mitchell"
};

class TaskResize : public Task {
public:
	unsigned int x_size = 0, y_size = 0;
	Interpolation interpolation = Interpolation::none;

	TaskResize();
	TaskResize(unsigned int x_size, unsigned int y_size, Interpolation interpolation);

	///"1: resize to 1920x1080 | Bilinear"
	QString to_string(unsigned short index) const override;
	///"resize to 1920x1080 | Bilinear"
	QString to_string() const override;

	OIIO::ImageBuf do_task(OIIO::ImageBuf input) override;
};
