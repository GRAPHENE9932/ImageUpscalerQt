/*
 * ImageUpscalerQt SRCNN task
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

#include <array>

#include "Task.h"

struct TaskSRCNN : public Task {
public:
	std::array<unsigned short, 3> kernels;
	std::array<unsigned short, 3> paddings;
	std::array<unsigned short, 2> channels;

	TaskSRCNN();
	TaskSRCNN(std::array<unsigned short, 3> kernels, std::array<unsigned short, 3> paddings,
			  std::array<unsigned short, 2> channels);

	///"1: use SRCNN 5-1-9 64-32"
	QString to_string(unsigned short index) const override;
	///"use SRCNN 5-1-9 64-32"
	QString to_string() const override;
	QString parameters_path() const;
	float progress() const override;

	OIIO::ImageBuf do_task(const OIIO::ImageBuf input) override;

private:
	long long blocks_amount;
	long long blocks_processed;
};
