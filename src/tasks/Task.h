/*
 * ImageUpscalerQt abstract task
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

#include <string>

#include <OpenImageIO/imagebuf.h>
#include <QString>

enum class TaskKind : unsigned char {
	resize,
	srcnn,
	fsrcnn,
	convert_color_space
};

class Task {
public:
	TaskKind task_kind;
	bool cancel_requested = false;

	virtual QString to_string(unsigned short index) const = 0;
	virtual QString to_string() const = 0;

	virtual float progress() const { return 0; };
	virtual OIIO::ImageBuf do_task(const OIIO::ImageBuf input) = 0;
};
